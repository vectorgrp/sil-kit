#include "IbExtensions.hpp"

// concrete type needed for interface
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

#include "ib/extensions/ITraceMessageSink.hpp"
#include "IbExtensionImpl/CreateMdf4tracing.hpp"
#include "IbExtensionImpl/CreateInstance.hpp"

#include <iostream>


namespace ib { namespace extensions {

struct MdfProxy final
    : public IbExtensionProxy<ITraceMessageSink>
{
    using IbExtensionProxy<ITraceMessageSink>::IbExtensionProxy;

    void Open(SinkType type, const std::string& outputPath) override
    {
        return _instance->Open(type, outputPath);
    }

    void Close() override
    {
        return _instance->Close();
    }

    auto GetLogger() const -> mw::logging::ILogger* override
    {
        return _instance->GetLogger();
    }

    auto Name() const -> const std::string & override
    {
        return _instance->Name();
    }

    virtual void Trace(
        Direction dir,
        const mw::EndpointAddress& address,
        std::chrono::nanoseconds timestamp,
        const TraceMessage& message)  override
    {
        return _instance->Trace(dir, address, timestamp, message);
    }
};

auto CreateMdf4tracing(ib::mw::logging::ILogger* logger,
        const std::string& sinkName,
        const cfg::Config& config)
    -> std::unique_ptr<ITraceMessageSink>
{
    return CreateInstance<IIbMdf4tracing, MdfProxy>
        ("vibe-mdf4tracing", config, logger, sinkName);
}

}//end namespace extensions
}//end namespace ib
