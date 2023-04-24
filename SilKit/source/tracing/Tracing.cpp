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

#include <sstream>

#include "CreateMdf4Tracing.hpp"
#include "PcapSink.hpp"
#include "Tracing.hpp"
#include "PcapReplay.hpp"

#include "ILogger.hpp"

#include "string_utils.hpp"

namespace SilKit {
namespace Tracing {

// Tracing

//! \brief Creates the ITraceMessageSink's as declared in the configuration.
auto CreateTraceMessageSinks(Services::Logging::ILogger* logger,
                             const Config::ParticipantConfiguration& participantConfig)
    -> std::vector<std::unique_ptr<ITraceMessageSink>>
{
    auto controllerUsesSink = [](const auto& name, const auto& controllers) {
        for (const auto& ctrl : controllers)
        {
            for (const auto& sinkName : ctrl.useTraceSinks)
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
    // However, the network simulator does not define controllers in its ParticipantConfiguration,
    // so we emit a warning.
    auto sinkInUse = [&participantConfig, &controllerUsesSink](const auto& name) {
        bool ok = false;
        ok |= controllerUsesSink(name, participantConfig.canControllers);
        ok |= controllerUsesSink(name, participantConfig.ethernetControllers);
        ok |= controllerUsesSink(name, participantConfig.flexrayControllers);
        ok |= controllerUsesSink(name, participantConfig.linControllers);
        ok |= controllerUsesSink(name, participantConfig.dataPublishers);
        ok |= controllerUsesSink(name, participantConfig.dataSubscribers);

        return ok;
    };

    std::vector<std::unique_ptr<ITraceMessageSink>> newSinks;
    for (const auto& sinkCfg : participantConfig.tracing.traceSinks)
    {
        if (!sinkInUse(sinkCfg.name))
        {
            Services::Logging::Warn(logger,
                "Tracing: the trace sink '{}' on participant '{}' is not referenced in the config, creating anyway!",
                sinkCfg.name, participantConfig.participantName);
        }

        switch (sinkCfg.type)
        {
        case Config::TraceSink::Type::Mdf4File:
        {
            //the `config' contains information about the links, which
            // will be useful when naming the MDF4 channels
            auto sink = CreateMdf4Tracing(participantConfig, logger, participantConfig.participantName, sinkCfg.name);
            sink->Open(SinkType::Mdf4File, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case Config::TraceSink::Type::PcapFile:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(SinkType::PcapFile, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case Config::TraceSink::Type::PcapPipe:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(SinkType::PcapNamedPipe, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        default: throw SilKitError("Unknown Sink Type");
        }
    }

    return newSinks;
}

auto CreateReplayFiles(Services::Logging::ILogger* logger, const Config::ParticipantConfiguration& participantConfig)
    -> std::map<std::string, std::shared_ptr<IReplayFile>>
{
    std::map<std::string, std::shared_ptr<IReplayFile>> replayFiles;

    for (const auto& source : participantConfig.tracing.traceSources)
    {
        switch (source.type)
        {
        case Config::TraceSource::Type::Mdf4File:
        {
            auto file = CreateMdf4Replay(participantConfig, logger, source.inputPath);
            replayFiles.insert({source.name, std::move(file)});
            break;
        }
        case Config::TraceSource::Type::PcapFile:
        {
            auto provider = PcapReplay{};
            auto file = provider.OpenFile(participantConfig, source.inputPath, logger);
            replayFiles.insert({source.name, std::move(file)});
            break;
        }
        case Config::TraceSource::Type::Undefined: //[[fallthrough]]
        default: throw SilKitError("CreateReplayFiles: unknown TraceSource::Type!");
        }
    }

    return replayFiles;
}

// Replaying utilities

bool HasReplayConfig(const Config::ParticipantConfiguration& cfg)
{
    return !cfg.tracing.traceSources.empty();
}

bool IsValidReplayConfig(const Config::Replay& config)
{
    return (config.direction != Config::Replay::Direction::Undefined) && !config.useTraceSource.empty();
}

} // namespace Tracing
} // namespace SilKit
