#include "ib/extensions/CreateExtension.hpp"

#include "IbExtensions.hpp"
#include "IbExtensionImpl/IIbRegistry.hpp"

#include <iostream>
#include <functional>

namespace ib { namespace extensions {

//The intention of using the proxy is to remove the deleters and other
//implementation details from the return value type of CreateIbRegistry.
template<typename InterfaceType>
class IbExtensionProxy
{
public:
    using unique_ptr = std::unique_ptr<InterfaceType,
          std::function<void(InterfaceType*)>>;

    IbExtensionProxy() = delete;

    IbExtensionProxy(unique_ptr instance)
        : _instance(std::move(instance))
    {
    }
    virtual ~IbExtensionProxy() = default;

protected:
    unique_ptr _instance;
};

struct IbRegistryProxy
    : public IbExtensionProxy<IIbRegistry>
    , public IIbRegistry
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
};


//the unique_ptr's deleter is not type-erased, so we have to use the
//IbRegistryProxy to hide the deleters and shared instances of the DLL.

auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>
{
    try {
        //try to load the extension by its undecorated DLL/so name
        //and cache a reference to it.
        static std::weak_ptr<IIbExtension> cachedDll;
        auto dllInst = cachedDll.lock();
        if (!dllInst)
        {
            dllInst = LoadExtension("vib-registry", config.extensionConfig);
            cachedDll = dllInst;
        }

        if (dllInst)
        {
            auto& factory =
                dynamic_cast<IIbRegistryFactory&>(*dllInst);

            auto* rawInstance = factory.Create(config);
            if (!rawInstance)
            {
                throw ExtensionError(
                        "IbRegistryFactory::Create() returned invalid pointer"
                );
            }

            //life cycle management: we need to call the proper deleter
            //and also keep a reference to dllInst, which has the DLL 
            //cleanup handler attached as a deleter.
            //Otherwise the dll will be unloaded while the IbRegistry is still
            //alive.
           
            IbRegistryProxy::unique_ptr instance(rawInstance,
                [dllInst](IIbRegistry* inst) {
                    auto& factory =
                        dynamic_cast<IIbRegistryFactory&>(*dllInst);
                    factory.Release(inst);
                }
            );

            return std::make_unique<IbRegistryProxy>(
                    std::move(instance)
            );
        }
    }
    catch (const ExtensionError& err)
    {
        std::cout << "ERROR loading IbRegistry: " << err.what() << std::endl;
        throw;
    }
    throw ExtensionError("Could not load vib-registry shared library. "
        "Please make sure it resides at the location specified in the search "
        "path hints in the extension configuration.");
}

}//end namespace extensions
}//end namespace ib
