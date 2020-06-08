#pragma once
#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/cfg/fwd_decl.hpp"

#include <string>

namespace ib { namespace extensions {
//forwards
class IIbExtension;

//! \brief Helper factory to instantiate Mdf4tracing
//         Required because we don't have control over the CTor, and also we
//         need to keep a reference to the DLL which we originate from.

class IIbMdf4tracing
{
public:
    // we keep a copy of our dll instance directly embedded. This saves us to
    // implement a proxy class as currently implemented in CreateIbRegistry.
    virtual auto Create(ib::mw::logging::ILogger* logger,
            const std::string& name,
            const cfg::Config& config,
            std::shared_ptr<IIbExtension> dllInstance)
       -> std::unique_ptr<ITraceMessageSink> = 0;

};

auto CreateMdf4tracing(ib::mw::logging::ILogger* logger, const std::string& sinkName, const cfg::Config& config)
    -> std::unique_ptr<ITraceMessageSink>;

}//end namespace extensions
}//end namespace ib
