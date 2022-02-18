// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <sstream>

#include "IbExtensions.hpp"
#include "DllCache.hpp"

namespace ib { namespace extensions {

//! \brief FactorySingleton creates an instance of the given interface.
// The underlying extension library is cached using a static DllCache instance,
// which keeps the shared library loaded during the lifetime of the calling process.

template<typename FactoryT> 
auto FactorySingleton(const std::string& extensionName, const cfg::v1::datatypes::Extensions& config)
    -> FactoryT&
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto& ibExtension = cache.Get(extensionName, config);
    try
    {
        auto& factory = dynamic_cast<FactoryT&>(ibExtension);
        return factory;
    }
    catch (const std::bad_cast& err)
    {
        std::stringstream msg;
        msg << "ERROR loading " << extensionName
            << ": " << err.what();
        std::cout << msg.str() << std::endl;
        throw ExtensionError(msg.str());
    }

}
}//end namespace extensions
}//end namespace ib
