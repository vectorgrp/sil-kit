#pragma once
#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/cfg/fwd_decl.hpp"

#include <string>

namespace ib { namespace extensions {
//forwards
class IIbExtension;

//! \brief Helper factory to instantiate Mdf4tracing
//         Required because we don't have control over the CTor when an extension is loaded.

class IIbMdf4tracing
{
public:
    
    virtual auto Create( const cfg::Config& config,
            ib::mw::logging::ILogger* logger,
            const std::string& name
        )
       -> std::unique_ptr<ITraceMessageSink> = 0;

};

auto CreateMdf4tracing(ib::mw::logging::ILogger* logger, const std::string& sinkName, const cfg::Config& config)
    -> std::unique_ptr<ITraceMessageSink>;

}//end namespace extensions
}//end namespace ib
