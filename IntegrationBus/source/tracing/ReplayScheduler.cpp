// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ReplayScheduler.hpp"

#include <string>
#include <cassert>
#include <chrono>

#include "ib/cfg/Config.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/sim/all.hpp"
#include "ib/extensions/string_utils.hpp"

#include "IReplayDataController.hpp"
#include "Tracing.hpp"

using namespace std::literals::chrono_literals;

namespace ib {
namespace tracing {

using namespace extensions;

namespace 
{ 

std::vector<std::string> splitString(std::string input,
    const std::string& separator)
{
    std::vector<std::string> tokens;
    for (auto i = input.find(separator); i != input.npos; i = input.find(separator))
    {
        tokens.emplace_back(std::move(input.substr(0, i)));
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
    // Meta data uses fixed terms from the MDF spec, see Config.hpp:MdfChannel and vibe-mdf4tracing
    value ChannelName() const
    {
        return Get("mdf/channel_name");
    }

    value ChannelSource() const
    {
        return Get("mdf/source_info_name");
    }

    value ChannelPath() const
    {
        return Get("mdf/source_info_path");
    }

    value GroupPath() const
    {
        return Get("mdf/channel_group_path");
    }

    value GroupSource() const
    {
        return Get("mdf/channel_group_name");
    }

    value GroupName() const
    {
        return Get("mdf/channel_group_acquisition_name");
    }


    value Separator() const
    {
        try
        {
            return Get("mdf/channel_group_path_separator");
        }
        catch(...)
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
    value VirtualBusNumber() const
    {
        return Get("mdf/virtual_bus_number");
    }

    value PcapVersion() const
    {
        return Get("pcap/version");
    }

    value PcapGmtToLocal() const
    {
        return Get("pcap/gmt_to_local");
    }
private:
    value Get(const std::string& name) const
    {
        return _metaInfos.at(name);
    }
    const std::map<std::string, std::string>& _metaInfos;
	//Used as default in VIB, CANoe
    const std::string _defaultSeparator{"/"};
};

TraceMessageType to_channelType(cfg::Link::Type linkType)
{
    switch (linkType)
    {
    case cfg::Link::Type::AnalogIo:
        return TraceMessageType::AnlogIoMessage;
    case cfg::Link::Type::DigitalIo:
        return TraceMessageType::DigitalIoMessage;
    case cfg::Link::Type::PatternIo:
        return TraceMessageType::PatternIoMessage;
    case cfg::Link::Type::PwmIo:
        return TraceMessageType::PwmIoMessage;
    case cfg::Link::Type::GenericMessage:
        return TraceMessageType::GenericMessage;
    case cfg::Link::Type::Ethernet:
        return TraceMessageType::EthFrame;
    case cfg::Link::Type::CAN:
        return TraceMessageType::CanMessage;
    case cfg::Link::Type::LIN:
        return TraceMessageType::LinFrame;
    case cfg::Link::Type::FlexRay:
        return TraceMessageType::FrMessage;
    default:
        throw std::runtime_error("Unknown channel Type");
    }
}

// Helper to match a channel by a cfg::MdfChannel identification supplied by the user
bool MatchMdfChannel(std::shared_ptr<IReplayChannel> channel, const cfg::MdfChannel& mdfId)
{
    const auto metaInfos = MetaInfos(*channel);
    bool result = true;

    auto isSetAndEqual = [&result](const auto lhs, const auto rhs)
    {
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

// Helper to identify Channel by its name in VIB format
bool MatchIbChannel(std::shared_ptr<IReplayChannel> channel, const std::string& linkName,
    const std::string& participantName, const std::string& controllerName)
{
    // The source info contains 'Link/Participant/Controller'
    const auto metaInfos = MetaInfos(*channel);
    auto tokens = splitString(metaInfos.ChannelSource(), metaInfos.Separator());
    const auto& link = tokens.at(0);
    const auto& participant = tokens.at(1);
    const auto& service = tokens.at(2);
    if (link == linkName
        && participant == participantName
        && service == controllerName)
    {
        return true;
    }
    return false;
}

// Helper to check if a user defined config has non-default values
bool HasMdfChannelSelection(const cfg::MdfChannel& mdf)
{
    // True if at least one member was set by user
    return mdf.channelName
        || mdf.channelSource
        || mdf.channelPath

        || mdf.groupName
        || mdf.groupSource
        || mdf.groupPath
        ;
}

// Find the MDF channel associated with the given participant/controller names and type
auto FindReplayChannel(ib::mw::logging::ILogger* log,
    IReplayFile* replayFile,
    const cfg::Replay& replayConfig,
    const std::string& controllerName,
    const std::string& participantName,
    const std::string& linkName,
    const ib::cfg::Link::Type linkType
    ) ->  std::shared_ptr<IReplayChannel>
{
    const auto type = to_channelType(linkType);
    for (auto channel : *replayFile)
    {
        if (replayFile->Type() == IReplayFile::FileType::PcapFile
            && channel->Type() == type)
        {
            // PCAP only has a single replay channel
            log->Info("Replay: using channel '{}' from '{}' on {}", channel->Name(), 
                replayFile->FilePath(), controllerName);
            return channel;
        }
        if (HasMdfChannelSelection(replayConfig.mdfChannel))
        {
            // User specifies a lookup for us
            if (MatchMdfChannel(channel, replayConfig.mdfChannel))
            {
                return channel;
            }
        }
        else
        {
            // VIB builtin channel lookup
            if (channel->Type() != type)
            {
                log->Trace("Replay: skipping channel '{}' of type {}", channel->Name(), to_string(channel->Type()));
                continue;
            }
            if (MatchIbChannel(channel, linkName, participantName, controllerName))
            {
                return channel;
            }
        }
    }
    return {};
}
} //end anonymous namespace

ReplayScheduler::ReplayScheduler(const cfg::Config& config,
    const cfg::Participant& participantConfig,
    std::chrono::nanoseconds tickPeriod,
    mw::IComAdapter* comAdapter,
    mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
    , _tickPeriod{tickPeriod}
{
    _log = _comAdapter->GetLogger();

    // If NetworkSimulator is inactive, configure the replay controllers
    if (participantConfig.networkSimulators.empty())
    {
        ConfigureControllers(config, participantConfig);
    }

    _startTime = _timeProvider->Now();
    _timeProvider->RegisterNextSimStepHandler(
        [this](auto now, auto duration)
        {
            ReplayMessages(now, duration);
        }
    );
}

void ReplayScheduler::ConfigureNetworkSimulators(const cfg::Config& config, const cfg::Participant& participantConfig,
    tracing::IReplayDataController& netSim)
{
    // Make sure we are only invoked once per participant
    for (const auto knownSimulator : _knownSimulators)
    {
        if (knownSimulator == participantConfig.name)
        {
            _log->Debug("NetworkSimulator {} is already configured.", participantConfig.name);
            return;
        }
    }
    _knownSimulators.push_back(participantConfig.name);

    // when using the participant time provider we have exact, absolute time stamps of simulated time
    const bool useAbsoluteTimestamps = participantConfig.participantController.has_value();


    auto replayFiles = tracing::CreateReplayFiles(_log, config, participantConfig);
    if (replayFiles.empty())
    {
        _log->Error("ReplayScheduler: cannot open replay files.");
        throw std::runtime_error("ReplayScheduler: cannot open replay files.");
    }

    // assign replay files to the bus simulator implementing the replay data controller interface
    for (const auto& simulator : participantConfig.networkSimulators)
    {
        for (const auto& linkName : simulator.simulatedLinks)
        {
            try
            {
                const auto& link = cfg::get_by_name(config.simulationSetup.links, linkName);
                //NB currently (v3.3.7) the NetworkSimulator uses the controller's endpoints to create trace channel source infos.
                // Thus, we try to attach a replay task for each of our simulated link's endpoints.
                auto replayFile = replayFiles.at(simulator.replay.useTraceSource);
                std::vector<std::shared_ptr<IReplayChannel>> replayChannels{replayFile->begin(), replayFile->end()};

                // The endpoints might have the same name, thus we attach the channels in order we encounter them,
                // and each channel is used at most once.
                auto pickChannel = 
                    [&replayChannels](const auto& linkName, const auto& participantName, const auto& controllerName, const auto linkType) {
                    const auto type = to_channelType(linkType);

                    for (auto it = replayChannels.begin(); it != replayChannels.end(); ++it)
                    {
                        auto channel = *it;
                        if (channel->Type() != type)
                        {
                            continue;
                        }
                        if (MatchIbChannel(channel, linkName, participantName, controllerName))
                        {
                            //make sure this channel is not shared among endpoints
                            replayChannels.erase(it);
                            return channel;
                        }
                    }
                    return std::shared_ptr<IReplayChannel>{};
                };


                for (const auto& endpoint : link.endpoints)
                {
                    auto tokens = splitString(endpoint, "/");
                    const auto& controllerName = tokens.at(1);

                    auto replayChannel = pickChannel(
                        linkName,
                        participantConfig.name,
                        controllerName,
                        link.type
                    );

                    if (!replayChannel)
                    {
                        _log->Warn("{}: could not find a replay channel!", simulator.name);
                        continue; // throw std::runtime_error("Could not find a replay channel");
                    }
                    if (replayChannel->NumberOfMessages() < 1)
                    {
                        _log->Warn("ReplayScheduler: skipping empty replay channel {}", replayChannel->Name());
                        continue;
                    }
                    ReplayTask task{};
                    task.replayReader = std::move(replayChannel->GetReader());
                    task.name = replayChannel->Name();
                    task.replayFile = std::move(replayFile);
                    task.controller = &netSim;
                    task.initialTime = useAbsoluteTimestamps ? 0ns : replayChannel->StartTime();

                    _replayTasks.emplace_back(std::move(task));

                    }
            }
            catch (const std::exception& ex)
            {
                _log->Warn("ReplayScheduler: Could not configure network simulator " + simulator.name
                    + ": " + ex.what());

            }
        }

    }
}


void ReplayScheduler::ConfigureControllers(const cfg::Config& config, const cfg::Participant& participantConfig)
{
    // create trace sources (aka IReplayFile)
    auto replayFiles = tracing::CreateReplayFiles(_log, config, participantConfig);
    if (replayFiles.empty())
    {
        _log->Error("ReplayScheduler: cannot open replay files.");
        throw std::runtime_error("ReplayScheduler: cannot open replay files.");
    }
    auto getLinkById = [&config](auto id)
    {
        for (const auto& link : config.simulationSetup.links)
        {
            if (link.id == id)
            {
                return link;
            }
        }
        throw std::runtime_error("Replay: cannot find replay with id=" + std::to_string(id));
    };
    // create controllers listed in config
    auto makeTasks = [=](auto& controllers, auto createMethod) {
        for (const auto& controllerConfig : controllers)
        {
            try {
                ReplayTask task{};

                auto createController = std::bind(createMethod, _comAdapter, std::placeholders::_1);
                auto* controller = createController(controllerConfig.name);

                if (controller == nullptr)
                    throw std::runtime_error("Create controller returned nullptr for "
                        + controllerConfig.name);

                auto& replayController = dynamic_cast<IReplayDataController&>(*controller);
                task.controller = &replayController;

                auto replayFile = replayFiles.at(controllerConfig.replay.useTraceSource);
                if (!replayFile)
                    throw std::runtime_error("No replay file found for" + 
                    controllerConfig.name);

                auto replayChannel = FindReplayChannel(
                    _log,
                    replayFile.get(),
                    controllerConfig.replay,
                    controllerConfig.name,
                    participantConfig.name,
                    getLinkById(controllerConfig.linkId).name,
                    controllerConfig.linkType
                );

                if (!replayChannel)
                {
                    _log->Warn("{}: could not find a replay channel!", controllerConfig.name);
                    continue; // throw std::runtime_error("Could not find a replay channel");
                }

                task.replayReader = std::move(replayChannel->GetReader());
                task.initialTime = replayChannel->StartTime();
                task.name = replayChannel->Name();
                task.replayFile = std::move(replayFile);

                _replayTasks.emplace_back(std::move(task));
            }
            catch (const std::runtime_error& ex)
            {
                _log->Warn("ReplayScheduler: Could not configure controller " + controllerConfig.name
                    + ": " + ex.what());
            }
        }
    };

    auto makePortTasks = [makeTasks](auto portList, auto inMethod, auto outMethod) {
        auto inPorts = decltype(portList){};
        auto outPorts = decltype(portList){};
        for (const auto& port : portList)
        {
            if (port.direction == cfg::PortDirection::In)
            {
                inPorts.push_back(port);
            }
            if (port.direction == cfg::PortDirection::Out)
            {
                outPorts.push_back(port);
            }
        }
        makeTasks(inPorts, inMethod);
        makeTasks(outPorts, outMethod);
    };

    // Bus Controllers
    makeTasks(participantConfig.ethernetControllers, &mw::IComAdapter::CreateEthController);
    makeTasks(participantConfig.canControllers, &mw::IComAdapter::CreateCanController);
    //TODO makeTasks(participantConfig.flexrayControllers, &mw::IComAdapter::CreateFlexrayController);
    makeTasks(participantConfig.linControllers, &mw::IComAdapter::CreateLinController);

    // Generic Messages
    makeTasks(participantConfig.genericPublishers, &mw::IComAdapter::CreateGenericPublisher);
    makeTasks(participantConfig.genericSubscribers, &mw::IComAdapter::CreateGenericSubscriber);

    // Ports
    makePortTasks(participantConfig.pwmPorts, &mw::IComAdapter::CreatePwmIn,
        &mw::IComAdapter::CreatePwmOut);
    makePortTasks(participantConfig.patternPorts, &mw::IComAdapter::CreatePatternIn,
        &mw::IComAdapter::CreatePatternOut);
    makePortTasks(participantConfig.digitalIoPorts, &mw::IComAdapter::CreateDigitalIn,
        &mw::IComAdapter::CreateDigitalOut);
    makePortTasks(participantConfig.analogIoPorts, &mw::IComAdapter::CreateAnalogIn,
        &mw::IComAdapter::CreateAnalogOut);

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

    const auto relativeNow = now - _startTime;
    assert(relativeNow.count() >= 0);
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
                _log->Trace("ReplayTask on channel '{}' returned invalid message @{}ns",
                    task.name, now.count());
                task.doneReplaying = true;
                break;
            }
            // the current time stamps are relative to the trace's initial time.
            const auto msgNow = msg->Timestamp() - task.initialTime;
            if (msgNow >= relativeEnd)
            {
                //message is after the current schedule
                break;
            }
            //TODO Currently, the messages are batched at the beginning of the schedule.
            //     When using wallclock time provider, the message timestamps might be off.
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

} //end namespace tracing
} //end namespace ib

