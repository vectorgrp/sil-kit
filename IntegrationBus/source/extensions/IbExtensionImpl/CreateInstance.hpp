// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <memory>
#include <sstream>

#include "IbExtensions.hpp"
#include "DllCache.hpp"

namespace ib { namespace extensions {

//! \brief CreateInstance creates an instance of the given interface.
// The underlying extension is cached as a a shared pointer, and stays
// loaded during the lifetime of the DllCache instance.

template<typename FactoryT> 
auto CreateInstance(const std::string& extensionName, const ib::cfg::Config& config) 
    -> FactoryT&
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto dll = cache.Get(extensionName, config);
    try
    {
        auto& factory = dynamic_cast<FactoryT&>(*dll);
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
