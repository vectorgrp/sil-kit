#include "IbExtensions.hpp"

// concrete type needed for interface
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "ib/sim/generic/GenericMessageDatatypes.hpp"
#include "ib/sim/io/IoDatatypes.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

#include "IbExtensionImpl/CreateMdf4tracing.hpp"

#include <iostream>

namespace {

//utilities

struct DllCache
{
    auto Get(const std::string& extensionName)
        -> std::shared_ptr<ib::extensions::IIbExtension>
    {
        try {
            //try to load the extension by its undecorated DLL/so name
            //and cache a reference to it.
            auto dllInst = _dll.lock();
            if (!dllInst)
            {
                dllInst = ib::extensions::LoadExtension(extensionName);
                _dll = dllInst;
                _extensionName = extensionName;
            }
            return dllInst;
        }
        catch (const ib::extensions::ExtensionError& err)
        {
            std::cout << "ERROR loading IbRegistry: " << err.what() << std::endl;
            throw;
        }
    }
    std::string _extensionName;
    std::weak_ptr<ib::extensions::IIbExtension> _dll;
};
} //anonymous namespace

namespace ib { namespace extensions {


auto CreateMdf4tracing(ib::mw::logging::ILogger* logger,
        const std::string& sinkName,
        const cfg::Config& config)
    -> std::unique_ptr<ITraceMessageSink>
{
    const std::string vibeName{"vibe-mdf4tracing"};
    static DllCache cache;

    auto dll = cache.Get(vibeName);
    if(!dll)
    {
        throw ExtensionError("Could not load " +  vibeName + "shared library."
            "Please make sure it is in the working directory.");
    }

    auto* mdf = dynamic_cast<IIbMdf4tracing*>(dll.get());
    if(mdf == nullptr)
    {
        throw ExtensionError(vibeName + " does not implement IIbMdf4Tracing"
                " C++ type!");
    }
    return mdf->Create(logger, sinkName, config,  std::move(dll));
}

}//end namespace extensions
}//end namespace ib
