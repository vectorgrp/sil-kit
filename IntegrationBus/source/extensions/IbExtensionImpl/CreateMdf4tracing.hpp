#pragma once
#include "ib/extensions/IIbRegistry.hpp"
#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include <string>

namespace ib { namespace extensions {

//! \brief Helper factory to instantiate Mdf4tracing
//         Required because we don't have controll over the CTor, and also we
//         need to keep a reference to the DLL which we originate from.

class IIbMdf4tracing  : public ITraceMessageSink
{
public:
    // we keep a copy of our dll instance directly embedded. This saves us to
    // implement a proxy class as in CreateIbRegistry.
    virtual auto Create(ib::mw::logging::ILogger* logger,
            const std::string& name, std::shared_ptr<IIbExtension> dllInstance)
       -> std::unique_ptr<ITraceMessageSink> = 0;

protected:
    std::shared_ptr<IIbExtension> _dllInstance;
};

auto CreateMdf4tracing(ib::mw::logging::ILogger* logger, const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>;

}//end namespace extensions
}//end namespace ib
