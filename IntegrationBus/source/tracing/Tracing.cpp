#include "Tracing.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "PcapSink.hpp"

#include "IbExtensionImpl/CreateMdf4tracing.hpp"

#include <sstream>

namespace ib {
namespace tracing {

//Utilities
std::ostream& operator<<(std::ostream& out, const TraceMessage& msg)
{
    out << "TraceMessage<"<< msg.QueryType()  << ">";
    return out;
}

std::ostream& operator<<(std::ostream& out, const TraceMessageType& type)
{
    using MsgT = TraceMessageType;
    switch (type)
    {
    case MsgT::Invalid:
        out << "Invalid"; break;
    case MsgT::EthFrame:
        out << "EthFrame"; break;
    case MsgT::CanMessage:
        out << "CanMessage"; break;
    case MsgT::LinFrame:
        out << "LinFrame"; break;
    case MsgT::GenericMessage:
        out << "GenericMessage"; break;
    case MsgT::AnlogIoMessage:
        out << "AnlogIoMessage"; break;
    case MsgT::DigitalIoMessage:
        out << "DigitalIoMessage"; break;
    case MsgT::PatternIoMessage:
        out << "PatternIoMessage"; break;
    case MsgT::PwmIoMessage:
        out << "PwmIoMessage"; break;
    case MsgT::FrMessage:
        out << "FrMessage"; break;
    default:
        throw std::runtime_error("Unknown TraceMessage::Type in operator<<!");
    }
    return out;
}

std::string to_string(const TraceMessageType& type)
{
    std::stringstream ss;
    ss << type;
    return ss.str();
}

std::string to_string(const TraceMessage& msg)
{
    std::stringstream ss;
    ss << msg;
    return ss.str();
}


void CreateTraceMessageSinks(mw::logging::ILogger* logger,
    cfg::Participant participantConfig,
    RegistrationCallbackT registerSink)
{
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
            auto sink = extensions::CreateMdf4tracing(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::Mdf4File, sinkCfg.outputPath);
            registerSink(std::move(sink));
            break;
        }
        case cfg::TraceSink::Type::PcapFile:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapFile, sinkCfg.outputPath);
            registerSink(std::move(sink));
            break;
        }
        case  cfg::TraceSink::Type::PcapPipe:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapNamedPipe, sinkCfg.outputPath);
            registerSink(std::move(sink));
            break;
        }
        default:
            throw std::runtime_error("Unknown Sink Type");
        }
    }

    //legacy configuration used pcapFile or pcapPipe fields 
    for (const auto& ethController : participantConfig.ethernetControllers)
    {
        if (!ethController.pcapFile.empty())
        {
            auto sink = std::make_unique<tracing::PcapSink>(logger, "pcapFile");
            sink->Open(tracing::SinkType::PcapFile, ethController.pcapFile);
            registerSink(std::move(sink));
        }

        if (!ethController.pcapPipe.empty())
        {
            auto sink = std::make_unique<tracing::PcapSink>(logger, "pcapFile");
            sink->Open(tracing::SinkType::PcapNamedPipe, ethController.pcapPipe);
            registerSink(std::move(sink));
        }
    }
}

} //end namespace tracing
} //end namespace ib

