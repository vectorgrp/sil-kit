// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "IbExtensions.hpp"
#include "IbExtensionImpl/IbExtensionProxy.hpp"
#include "IbExtensionImpl/DllCache.hpp"

namespace ib { namespace extensions {

//! \brief CreateInstance creates an instance of the given interface bundled with a shared pointer to the underlying shared library
// This simplifies the usage for the actual Create`Interface`(`CTor  arguments`) 
// factories.
// NB this is only needed because we don't want to keep a global list of opened 
//    shared libraries and also adds a layer of indirection through the Proxy instance.

template<typename FactoryT, typename ProxyT, typename... Arg>
auto CreateInstance(const std::string& extensionName,
    const cfg::Config& config, Arg&&... args) -> std::unique_ptr<ProxyT>
{
    static DllCache cache;
    try
    {
        auto dll = cache.Get(extensionName, config);
        auto& factory = dynamic_cast<FactoryT&>(*dll);
        auto instance = factory.Create(config, std::forward<Arg>(args)...);
        return std::make_unique<ProxyT>(std::move(instance), std::move(dll));
    }
    catch (const ExtensionError& err)
    {
        std::cout << "ERROR loading " << extensionName 
            << ": " << err.what() << std::endl;
        throw;
    }

}
}//end namespace extensions
}//end namespace ib
