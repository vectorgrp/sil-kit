// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <sstream>

#include "ib/cfg/Config.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "CreateMdf4Tracing.hpp"

#include "PcapSink.hpp"
#include "string_utils.hpp"
#include "Tracing.hpp"

namespace ib {
namespace tracing {

using namespace extensions;
//string_utils
std::ostream& operator<<(std::ostream& out, const TraceMessage& msg)
{
    return out << "TraceMessage<"<< msg.Type()  << ">";
}

std::string to_string(const TraceMessage& msg)
{
    std::stringstream ss;
    ss << msg;
    return ss.str();
}

std::ostream& operator<<(std::ostream& out, TraceMessageType type)
{
    return out << to_string(type);
}

std::string to_string(TraceMessageType type)
{
    switch (type)
    {
    case TraceMessageType::EthFrame: return "EthFrame"; 
    case TraceMessageType::CanMessage: return "CanMessage"; 
    case TraceMessageType::LinFrame: return "LinFrame"; 
    case TraceMessageType::GenericMessage: return "GenericMessage"; 
    case TraceMessageType::AnlogIoMessage: return "AnlogIoMessage"; 
    case TraceMessageType::DigitalIoMessage: return "DigitalIoMessage"; 
    case TraceMessageType::PatternIoMessage: return "PatternIoMessage"; 
    case TraceMessageType::PwmIoMessage: return "PwmIoMessage";
    case TraceMessageType::FrMessage: return "FrMessage";
    default:
        throw std::runtime_error("Unknown TraceMessage::Type in operator<<!");
    }
}


// Tracing

auto CreateTraceMessageSinks(
    mw::logging::ILogger* logger,
    const cfg::Config& config,
    const cfg::Participant& participantConfig
    ) -> std::vector<std::unique_ptr<ITraceMessageSink>>
{
    std::vector<std::unique_ptr<ITraceMessageSink>> newSinks;
    for (const auto& sinkCfg : participantConfig.traceSinks)
    {
        if (!sinkCfg.enabled)
        {
            logger->Debug("Tracing: skipping disabled sink {} on participant {}",
                sinkCfg.name, participantConfig.name);
            continue;
        }

        switch (sinkCfg.type)
        {
        case cfg::TraceSink::Type::Mdf4File:
        {
            //the `config' contains information about the links, which
            // will be useful when naming the MDF4 channels
            auto sink = extensions::CreateMdf4Tracing(config, logger, sinkCfg.name);
            sink->Open(tracing::SinkType::Mdf4File, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case cfg::TraceSink::Type::PcapFile:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapFile, sinkCfg.outputPath);
            newSinks.emplace_back(std::move(sink));
            break;
        }
        case  cfg::TraceSink::Type::PcapPipe:
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

} //end namespace tracing
} //end namespace ib
