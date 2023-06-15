/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "ReplayScheduler.hpp"

#include <string>
#include <chrono>
#include <sstream>

#include "silkit/participant/IParticipant.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"
#include "silkit/services/all.hpp"

#include "IParticipantInternal.hpp"
#include "IReplayDataController.hpp"
#include "Tracing.hpp"
#include "Assert.hpp"
#include "Logger.hpp"
#include "string_utils.hpp"

using namespace std::literals::chrono_literals;

namespace SilKit {
namespace Tracing {

namespace {

TraceMessageType ToTraceMessageType(Config::NetworkType networkType)
{
    switch (networkType)
    {
    case Config::NetworkType::Ethernet: return TraceMessageType::EthernetFrame;
    case Config::NetworkType::CAN: return TraceMessageType::CanFrameEvent;
    case Config::NetworkType::LIN: return TraceMessageType::LinFrame;
    case Config::NetworkType::FlexRay: return TraceMessageType::FlexrayFrameEvent;
    case Config::NetworkType::Data: return TraceMessageType::DataMessageEvent;
    default: throw SilKitError("Unknown channel Type");
    }
}

std::vector<std::string> splitString(std::string input, const std::string& separator)
{
    std::vector<std::string> tokens;
    for (auto i = input.find(separator); i != input.npos; i = input.find(separator))
    {
        tokens.emplace_back(input.substr(0, i));
        input = input.substr(i + 1);
    }
    if (!input.empty())
    {
        tokens.emplace_back(std::move(input));
    }
    return tokens;
}

class MetaInfos
{
public:
    using value = const std::string&;
    MetaInfos(const IReplayChannel& channel)
        : _metaInfos{channel.GetMetaInfos()}
    {
    }

    // Meta data uses fixed terms from the MDF spec, see Config.hpp:MdfChannel and SilKitExtension_Mdf
    value ChannelName() const { return Get("mdf/channel_name"); }

    value ChannelSource() const { return Get("mdf/source_info_name"); }

    value ChannelPath() const { return Get("mdf/source_info_path"); }

    value GroupPath() const { return Get("mdf/channel_group_path"); }

    value GroupSource() const { return Get("mdf/channel_group_name"); }

    value GroupName() const { return Get("mdf/channel_group_acquisition_name"); }

    value Separator() const
    {
        try
        {
            return Get("mdf/channel_group_path_separator");
        }
        catch (...)
        {
            return _defaultSeparator;
        }
    }

    // available since >v3.3.8, returns 0 if the underlying meta infos does not contain an absolute (trace) start time
    std::chrono::nanoseconds AbsoluteStartTime() const
    {
        auto it = _metaInfos.find("mdf/absolute_start_time");
        if (it != _metaInfos.end())
        {
            return std::chrono::nanoseconds{std::stoll(it->second)};
        }
        return std::chrono::nanoseconds{0};
    }

    value VirtualBusNumber() const { return Get("mdf/virtual_bus_number"); }

    value PcapVersion() const { return Get("pcap/version"); }

    value PcapGmtToLocal() const { return Get("pcap/gmt_to_local"); }

private:
    value Get(const std::string& name) const { return _metaInfos.at(name); }

private:
    const std::map<std::string, std::string>& _metaInfos;
    //Used as default in SILKIT, CANoe
    const std::string _defaultSeparator{"/"};
};

// Helper to match a channel by a Config::MdfChannel identification supplied by the user
bool MatchMdfChannel(std::shared_ptr<IReplayChannel> channel, const Config::MdfChannel& mdfId)
{
    const auto metaInfos = MetaInfos(*channel);
    bool result = true;

    auto isSetAndEqual = [&result](const auto lhs, const auto rhs) {
        if (lhs.has_value())
        {
            result &= (lhs.value() == rhs);
        }
    };

    isSetAndEqual(mdfId.channelName, metaInfos.ChannelName());
    isSetAndEqual(mdfId.channelPath, metaInfos.ChannelPath());
    isSetAndEqual(mdfId.channelSource, metaInfos.ChannelSource());
    isSetAndEqual(mdfId.groupName, metaInfos.GroupName());
    isSetAndEqual(mdfId.groupPath, metaInfos.GroupPath());
    isSetAndEqual(mdfId.groupSource, metaInfos.GroupSource());

    return result;
}

// Helper to identify Channel by its name in SILKIT format
bool MatchSilKitChannel(std::shared_ptr<IReplayChannel> channel, const std::string& networkName,
                        const std::string& participantName, const std::string& controllerName)
{
    // The source info contains 'Link/Participant/Controller'
    const auto metaInfos = MetaInfos(*channel);
    auto tokens = splitString(metaInfos.ChannelSource(), "/");
    const auto& link = tokens.at(0);
    const auto& participant = tokens.at(1);
    const auto& service = tokens.at(2);
    if (link == networkName && participant == participantName && service == controllerName)
    {
        return true;
    }
    return false;
}

// Helper to check if a user defined config has non-default values
bool HasMdfChannelSelection(const Config::MdfChannel& mdf)
{
    // True if at least one member was set by user
    return mdf.channelName || mdf.channelSource || mdf.channelPath

           || mdf.groupName || mdf.groupSource || mdf.groupPath;
}

// Helper for useful error messages
std::string to_string(const Config::MdfChannel& mdf)
{
    std::stringstream result;
    result << "MdfChannel{";
    auto printField = [&result](const auto& name, const auto field) {
        if (field.has_value())
        {
            result << name << ": "
                   << "\"" << field.value() << "\", ";
        }
    };
    printField("ChannelName", mdf.channelName);
    printField("ChannelSource", mdf.channelSource);
    printField("ChannelPath", mdf.channelPath);
    printField("GroupName", mdf.groupName);
    printField("GroupSource", mdf.groupSource);
    printField("GroupPath", mdf.groupPath);
    result << "}";
    return result.str();
}

// Find the MDF channels associated with the given participant/controller names and types or an MdfChannel identification.
auto FindReplayChannel(SilKit::Services::Logging::ILogger* log, IReplayFile* replayFile,
                       const Config::Replay& replayConfig, const std::string& controllerName,
                       const std::string& participantName, const std::string& networkName,
                       const Config::NetworkType networkType) -> std::shared_ptr<IReplayChannel>
{

    std::vector<std::shared_ptr<IReplayChannel>> channelList;

    const auto type = ToTraceMessageType(networkType);
    for (auto channel : *replayFile)
    {
        if (replayFile->Type() == IReplayFile::FileType::PcapFile && channel->Type() == type)
        {
            // PCAP only has a single replay channel
            Services::Logging::Info(log, "Replay: using channel '{}' from '{}' on {}", channel->Name(),
                                    replayFile->FilePath(), controllerName);
            return channel;
        }

        if (HasMdfChannelSelection(replayConfig.mdfChannel))
        {
            // User specifies lookup information for us
            if (MatchMdfChannel(channel, replayConfig.mdfChannel))
            {
                channelList.emplace_back(std::move(channel));
            }
        }
        else
        {
            // SILKIT builtin channel lookup
            if (channel->Type() != type)
            {
                Services::Logging::Trace(log, "Replay: skipping channel '{}' of type {}", channel->Name(),
                                         to_string(channel->Type()));
                continue;
            }
            if (MatchSilKitChannel(channel, networkName, participantName, controllerName))
            {
                Services::Logging::Debug(log, "Replay: found channel '{}' from file '{}' for type {}",
                    channel->Name(), replayFile->FilePath(), to_string(channel->Type()));
                channelList.emplace_back(std::move(channel));

            }
        }
    }

    // when an MdfChannel config is given, the channel has to be unique.
    if (HasMdfChannelSelection(replayConfig.mdfChannel) && (channelList.size() != 1))
    {
        std::stringstream msg;
        msg << "Error in MDF channel selection: the config of " << to_string(replayConfig.mdfChannel) << " found "
            << channelList.size() << " channels in \"" << replayFile->FilePath() << "\"."
            << " MdfChannel config must yield a unique channel!";
        throw SilKit::ConfigurationError{msg.str()};
    }

    if (channelList.size() < 1)
    {
        return {};
    }
    return channelList.at(0);
}

} // namespace

ReplayScheduler::ReplayScheduler(const Config::ParticipantConfiguration& participantConfiguration,
                                 Core::IParticipantInternal* participant)
    : _participant{participant}
{
    _log = _participant->GetLogger();

    CreateReplayFiles(participantConfiguration);
}

void ReplayScheduler::ConfigureTimeProvider(Services::Orchestration::ITimeProvider* timeProvider)
{
    // NB: The time provider switches its implementation internally. The registered NextSimStepHandlers are moved
    //     to the other implementation during this switch. Therefore no re-registration is required.
    //     This switch occurs, when the time-sync service is activated, the default time provider is the wallclock.

    _timeProvider = timeProvider;

    _timeProvider->AddNextSimStepHandler([this](auto now, auto duration) {
        ReplayMessages(now, duration);
    });
}

void ReplayScheduler::ConfigureController(const std::string& controllerName, IReplayDataController* replayController,
                                          const Config::Replay& replayConfig, const std::string& networkName,
                                          const Config::NetworkType networkType)
{
    try
    {
        ReplayTask task{};

        // Not all controllers might have active replaying -- we only know that at least one
        // controller has replaying active.
        if (!IsValidReplayConfig(replayConfig))
        {
            Services::Logging::Debug(_log,
                                     "ReplayScheduler::ConfigureController: skipping controller {} because it has no "
                                     "active Replay!",
                                     controllerName);
            return;
        }

        task.controller = replayController;

        auto replayFile = _replayFiles.at(replayConfig.useTraceSource);
        if (!replayFile)
            throw SilKitError("No replay file found for" + controllerName);

        auto replayChannel = FindReplayChannel(_log, replayFile.get(), replayConfig, controllerName,
                                               _participant->GetParticipantName(), networkName, networkType);

        if (!replayChannel)
        {
            Services::Logging::Warn(_log, "{}: could not find a replay channel!", controllerName);
            throw SilKitError("Could not find a replay channel");
        }

        task.replayReader = replayChannel->GetReader();
        task.initialTime = replayChannel->StartTime();
        task.name = replayChannel->Name();
        task.replayFile = std::move(replayFile);

        _replayTasks.emplace_back(std::move(task));
    }
    catch (const SilKit::ConfigurationError& ex)
    {
        _log->Error("ReplayScheduler: misconfiguration of controller " + controllerName + ": " + ex.what());
        throw;
    }
    catch (const SilKitError& ex)
    {
        _log->Warn("ReplayScheduler: Could not configure controller " + controllerName + ": " + ex.what());
    }
}

void ReplayScheduler::CreateReplayFiles(const Config::ParticipantConfiguration& participantConfiguration)
{
    // create trace sources (aka IReplayFile)
    _replayFiles = Tracing::CreateReplayFiles(_log, participantConfiguration);
    if (_replayFiles.empty())
    {
        _log->Error("ReplayScheduler: cannot open replay files.");
        throw SilKitError("ReplayScheduler: cannot open replay files.");
    }
}

ReplayScheduler::~ReplayScheduler()
{
    _isDone = true;
}

void ReplayScheduler::ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    if (_isDone)
    {
        return;
    }

    if (!_timeProvider->IsSynchronizingVirtualTime())
    {
        // we are still attached to a NoSync timeProvider
        return;
    }

    if (_startTime == std::chrono::nanoseconds::min())
    {
        _startTime = _timeProvider->Now();
    }

    const auto relativeNow = now - _startTime;
    SILKIT_ASSERT(relativeNow.count() >= 0);
    const auto relativeEnd = relativeNow + duration;
    for (auto& task : _replayTasks)
    {
        if (task.doneReplaying)
        {
            continue;
        }

        while (true)
        {
            auto msg = task.replayReader->Read();
            if (!msg)
            {
                Services::Logging::Trace(_log, "ReplayTask on channel '{}' returned invalid message @{}ns", task.name,
                                         now.count());
                task.doneReplaying = true;
                break;
            }

            const auto msgNow = msg->Timestamp();
            if (msgNow >= relativeEnd)
            {
                //message is after the current schedule
                break;
            }

            //NB: Currently, the messages are batched at the beginning of the schedule.
            //    When using wallclock time provider, the message timestamps might be off.
            task.controller->ReplayMessage(msg.get());

            if (!task.replayReader->Seek(1))
            {
                // we're at the end of the replay channel
                task.doneReplaying = true;
                break;
            }
        }
    }
}

} // namespace Tracing
} // namespace SilKit
