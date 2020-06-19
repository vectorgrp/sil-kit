// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "IbExtensions.hpp"
#include "IbExtensionImpl/IbExtensionProxy.hpp"
#include "IbExtensionImpl/DllCache.hpp"

namespace ib { namespace extensions {

//! \brief CreateInstance creates an instance of the given interface bundled with a shared pointer to the underlying shared library
// NB this simplifies housekeeping of opened/used shared libraries at the cost of adding a layer of indirections
//    through the Proxy interface

template<typename FactoryT, typename ProxyT, typename... Arg>
auto CreateInstance(const std::string& extensionName,
    cfg::Config config, Arg&&... args) -> std::unique_ptr<ProxyT>
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto dll = cache.Get(extensionName, config);
    try
    {
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
