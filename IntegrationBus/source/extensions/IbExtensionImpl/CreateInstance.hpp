// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <memory>

#include "IbExtensions.hpp"
#include "DllCache.hpp"

namespace ib { namespace extensions {

//! \brief CreateInstance creates an instance of the given interface.
// The underlying extension is cached as a a shared pointer, and stays
// loaded during the lifetime of the DllCache instance.

template<typename FactoryT, typename ExtensionInterfaceT, typename... Arg>
auto CreateInstance(const std::string& extensionName,
    cfg::Config config, Arg&&... args) -> std::unique_ptr<ExtensionInterfaceT>
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto dll = cache.Get(extensionName, config);
    try
    {
        auto& factory = dynamic_cast<FactoryT&>(*dll);
        return factory.Create(config, std::forward<Arg>(args)...);
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
