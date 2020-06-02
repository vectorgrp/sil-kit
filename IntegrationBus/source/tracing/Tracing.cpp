#include "Tracing.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "PcapSink.hpp"

namespace ib {
namespace tracing {


// TODO implement plugin loading and creation of TraceSinks here
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

            throw std::runtime_error("SinkType Mdf4File not implemented yet!");
        }
        case cfg::TraceSink::Type::PcapFile:
        {
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapFile, sinkCfg.outputPath);
            registerSink(std::move(sink));
            break;
        }
        case  cfg::TraceSink::Type::PcapPipe:
            auto sink = std::make_unique<PcapSink>(logger, sinkCfg.name);
            sink->Open(tracing::SinkType::PcapNamedPipe, sinkCfg.outputPath);
            registerSink(std::move(sink));
            break;
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

