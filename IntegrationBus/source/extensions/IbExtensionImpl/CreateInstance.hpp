// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "ib/extensions/ExtensionHandle.hpp"

#include "IbExtensions.hpp"
#include "DllCache.hpp"

namespace ib { namespace extensions {

//! \brief CreateInstance creates an instance of the given interface bundled with a shared pointer to the underlying shared library
// NB this simplifies housekeeping of opened/used shared libraries at the cost of adding a layer of indirections
//    through the Proxy interface

template<typename FactoryT, typename ExtensionInterfaceT, typename... Arg>
auto CreateInstance(const std::string& extensionName,
    cfg::Config config, Arg&&... args) -> ExtensionHandle<ExtensionInterfaceT>
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto dll = cache.Get(extensionName, config);
    try
    {
        auto& factory = dynamic_cast<FactoryT&>(*dll);
        auto instance = factory.Create(config, std::forward<Arg>(args)...);
        return ExtensionHandle<ExtensionInterfaceT>(std::move(dll), std::move(instance));
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
