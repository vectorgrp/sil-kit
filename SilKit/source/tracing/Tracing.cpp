// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <sstream>

#include "silkit/config/Config.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/extensions/string_utils.hpp"

#include "CreateMdf4Tracing.hpp"
#include "PcapSink.hpp"
#include "Tracing.hpp"
#include "PcapReplay.hpp"

namespace SilKit {

namespace tracing {

using ITraceMessageSink;
using SinkType;
using TraceMessage;
using TraceMessageType;



// Tracing

auto CreateTraceMessageSinks(
    Services::Logging::ILogger* logger,
    const Config::Config& config,
    const Config::Participant& participantConfig
    ) -> std::vector<std::unique_ptr<ITraceMessageSink>>
{
    auto controllerUsesSink = [&participantConfig](const auto& name, const auto& controllers)
    {
        for (const auto& ctrl : controllers)
        {
            for (const auto&  sinkName: ctrl.useTraceSinks)
            {
                if (sinkName == name)
                {
                    return true;
                }
            }
        }
        return false;
    };

    // Trace sinks should only be instantiated if they are used.
    // This solves a problem where a stale "TraceSinks" declaration in the config and an active Replay
    // configuration both access the same output/input file: the trace sink would truncate
    // the file to 0 bytes.
    auto sinkInUse = [&participantConfig, &controllerUsesSink](const auto& name)
    {
        bool ok = false;
        ok |= controllerUsesSink(name, participantConfig.canControllers);
        ok |= controllerUsesSink(name, participantConfig.ethernetControllers);
        ok |= controllerUsesSink(name, participantConfig.flexrayControllers);
        ok |= controllerUsesSink(name, participantConfig.linControllers);
        ok |= controllerUsesSink(name, participantConfig.genericPublishers);
        ok |= controllerUsesSink(name, participantConfig.genericSubscribers);
        ok |= controllerUsesSink(name, participantConfig.networkSimulators);

        return ok;
    };

    std::vector<std::unique_ptr<ITraceMessageSink>> newSinks;
    for (const auto& sinkCfg : participantConfig.traceSinks)
    {
        if (!sinkCfg.enabled || !sinkInUse(sinkCfg.name))
        {
            logger->Debug("Tracing: skipping disabled sink {} on participant {}",
                sinkCfg.name, participantConfig.name);
            continue;
        }

        switch (sinkCfg.type)
        {
        case Config::TraceSink::Type::Mdf4File:
        {
            //the `config' contains information about the links, which
            // will be useful when naming the MDF4 channels
            auto sink = CreateMdf4Tracing(config, logger, participantConfig.name, sinkCfg.name);
            sink->Open(tracing::SinkType::Mdf4File, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case Config::TraceSink::Type::PcapFile:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapFile, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case  Config::TraceSink::Type::PcapPipe:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapNamedPipe, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        default:
            throw std::runtime_error("Unknown Sink Type");
        }
    }

    return newSinks;
}

    
auto CreateReplayFiles(Services::Logging::ILogger* logger, /*const Config::Config& config,*/
    const Config::Participant& participantConfig)
    -> std::map<std::string, std::shared_ptr<IReplayFile>>
{
    std::map<std::string, std::shared_ptr<IReplayFile>> replayFiles;

    for (const auto& source : participantConfig.traceSources)
    {
        switch (source.type)
        {
        case Config::TraceSource::Type::Mdf4File:
        {
            auto file = CreateMdf4Replay(config, logger, source.inputPath);
            replayFiles.insert({source.name, std::move(file)});
            break;
        }
        case Config::TraceSource::Type::PcapFile:
        {
            auto provider = PcapReplay{};
            auto file = provider.OpenFile(/*config, */source.inputPath, logger);
            replayFiles.insert({source.name, std::move(file)});
            break;
        }
        case Config::TraceSource::Type::Undefined: //[[fallthrough]]
        default:
            throw std::runtime_error("CreateReplayFiles: unknown TraceSource::Type!");
        }
    }

    return replayFiles;
}

// Replaying utilities

bool HasReplayConfig(const Config::Participant& cfg)
{
    // if there are no replay trace sources, the Replay blocks are invalid
    if (cfg.traceSources.empty())
        return false;

    //find replay blocks of services
    bool ok = false;
    auto isActive = [&ok, &cfg](const auto& ctrls)
    {
        for (const auto& ctrl : ctrls)
        {
            if ( (ctrl.replay.direction != Config::Replay::Direction::Undefined)
                && !ctrl.replay.useTraceSource.empty()
            )
            {
                ok = true;
                break;
            }
        }
    };

    //Bus controllers
    isActive(cfg.canControllers);
    isActive(cfg.ethernetControllers);
    isActive(cfg.linControllers);
    isActive(cfg.flexrayControllers);
    // Generic Messages
    isActive(cfg.genericPublishers);
    isActive(cfg.genericSubscribers);
    // Network Simulator
    isActive(cfg.networkSimulators);

    return ok;
}
} //end namespace tracing
} //end namespace SilKit
