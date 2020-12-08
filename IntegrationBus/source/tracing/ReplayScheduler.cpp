// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ReplayScheduler.hpp"

#include <string>

#include "ib/cfg/Config.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/sim/all.hpp"
#include "ib/extensions/string_utils.hpp"

#include "IReplayDataController.hpp"
#include "Tracing.hpp"

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
    MetaInfos(IReplayChannel& channel)
    : _metaInfos{channel.GetMetaInfos()}
    {
    }
    value ChannelName() const
    {
        return Get("mdf/channel_name");
    }

    value ChannelGroupPath() const
    {
        return Get("mdf/channel_group_path");
    }

    value ChannelGroupName() const
    {
        return Get("mdf/channel_group_name");
    }

    value ChannelGroupAcquisitionName() const
    {
        return Get("mdf/channel_group_acquisition_name");
    }

    value SourceInfoName() const
    {
        return Get("mdf/source_info_name");
    }

    value SourceInfoPath() const
    {
        return Get("mdf/source_info_path");
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

template<typename ConfigT>
auto FindReplayChannel(ib::mw::logging::ILogger* log,
    IReplayFile* replayFile,
    const ConfigT& controllerConfig,
    const std::string& participantName,
    const std::string& linkName ) ->  std::shared_ptr<IReplayChannel>
{
    const auto type = to_channelType(controllerConfig.linkType);
    for (auto channel : *replayFile)
    {
        if (replayFile->Type() == IReplayFile::FileType::PcapFile
            && channel->Type() == type)
        {
            // PCAP only has a single replay channel
            log->Info("Replay: using channel '{}' from '{}' on {}", channel->Name(), replayFile->FilePath(), controllerConfig.name);
            return channel;
        }
        // TODO  MDF is currently hard-coded to the CANoe/VIB MDF usage.
        //       We have to revisit this when we have more user-configurable channel selections.
        if (channel->Type() != type)
        {
            log->Trace("Replay: skipping channel '{}' of type {}", channel->Name(), to_string(channel->Type()));
            continue;
        }
        // The source info contains 'Link/Participant/Controller'
        auto metaInfos = MetaInfos(*channel);
        auto tokens = splitString(metaInfos.SourceInfoName(), "/"); //TODO separator is a MDF detail
        if (tokens.size() != 3)
        {
            log->Info("Replay: using channel '{}' from '{}' on {}: source info mismatch: {}",
                channel->Name(), replayFile->FilePath(), controllerConfig.name, metaInfos.SourceInfoName());
            continue;
        }
        const auto& link = tokens.at(0);
        const auto& participant = tokens.at(1);
        const auto& service = tokens.at(2);
        if (link == linkName
            && participant == participantName
            && service == controllerConfig.name)
        {
            return channel;
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
    ConfigureControllers(config, participantConfig);
    _timeProvider->RegisterNextSimStepHandler(
        [this](auto now, auto duration)
        {
            ReplayMessages(now, duration);
        }
    );

}

void ReplayScheduler::ConfigureControllers(const cfg::Config& config, const cfg::Participant& participantConfig)
{
    auto* log = _comAdapter->GetLogger();
    // create trace sources (aka IReplayFile)
    auto replayFiles = tracing::CreateReplayFiles(log, config, participantConfig);
    if (replayFiles.empty())
    {
        log->Error("ReplayScheduler: cannot create replay files.");
        throw std::runtime_error("ReplayScheduler: cannot create replay files.");
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

                auto* replayController = dynamic_cast<IReplayDataController*>(controller);
                task.controller = replayController;

                auto replayFile = replayFiles.at(controllerConfig.replay.useTraceSource);
                if (!task.replayChannel)
                    throw std::runtime_error("No replay file found for" + 
                    controllerConfig.name);

                task.replayChannel = FindReplayChannel(
                    log,
                    replayFile.get(),
                    controllerConfig,
                    participantConfig.name,
                    getLinkById(controllerConfig.linkId).name
                );

                if (!task.replayChannel)
                    throw std::runtime_error("Could not find a replay channel");

                task.initialTime = task.replayChannel->StartTime();

                _replayTasks.emplace_back(std::move(task));
            }
            catch (const std::runtime_error& ex)
            {
                log->Warn("Could not configure controller " + controllerConfig.name
                    + ": " + ex.what());
            }
        }
    };
    makeTasks(participantConfig.ethernetControllers, &mw::IComAdapter::CreateEthController);
    /*
    makeTasks(participantConfig.canControllers);
    makeTasks(participantConfig.flexrayControllers);
    makeTasks(participantConfig.linControllers);
    makeTasks(participantConfig.pwmPorts);
    makeTasks(participantConfig.patternPorts);
    makeTasks(participantConfig.digitalIoPorts);
    makeTasks(participantConfig.analogIoPorts);
    makeTasks(participantConfig.genericPublishers);
    makeTasks(participantConfig.genericSubscribers);
    */
}

void ReplayScheduler::ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    const auto end = now + duration;
    for (auto& task : _replayTasks)
    {
        //TODO 
        // - play all messages that have a relative timestamp that is in our [now, now+duration] time frame
        // - if we get a replaymessage with higher relative timestamp, put it in currentMessage and continue to next task
    }
}

} //end namespace tracing
} //end namespace ib

