#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IbExtensionImpl/IIbRegistry.hpp"
#include "IbExtensionImpl/CreateInstance.hpp"

#include <iostream>
#include <functional>

namespace ib { namespace extensions {

struct IbRegistryProxy
    : public IbExtensionProxy<IIbRegistry>
{
    using IbExtensionProxy<IIbRegistry>::IbExtensionProxy;

    void ProvideDomain(uint32_t domainId)
    {
        return _instance->ProvideDomain(domainId);
    }

    void SetAllConnectedHandler(std::function<void()> handler)
    {
        return _instance->SetAllConnectedHandler(handler);
    }

    void SetAllDisconnectedHandler(std::function<void()> handler)
    {
        return _instance->SetAllDisconnectedHandler(handler);
    }

    auto GetLogger() -> mw::logging::ILogger*
    {
        return _instance->GetLogger();
    }
};


//the unique_ptr's deleter is not type-erased, so we have to use the
//IbRegistryProxy to hide the deleters and shared instances of the DLL.

auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>
{
    return CreateInstance<IIbRegistryFactory2, IbRegistryProxy>
        ("vib-registry", config);
}

}//end namespace extensions
}//end namespace ib
