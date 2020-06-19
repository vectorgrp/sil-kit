// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

namespace ib { namespace extensions {

//forwards
class IIbExtension;

//! \brief IbExtensionProxy implements the given extension interface and 
//         takes ownership of the underlying shared library.
// The intention of using the proxy is to ensure that the extension's destructors
// are run outside of the actual extension's memory / text segments.
//
// Usage:
// see CreateIbRegistry() and CreateMdf4tracing()

template<typename InterfaceType>
class IbExtensionProxy
    : public InterfaceType
{
public:
    IbExtensionProxy() = delete;
    IbExtensionProxy(IbExtensionProxy&&) = delete;

    IbExtensionProxy(std::unique_ptr<InterfaceType> instance,
        std::shared_ptr<IIbExtension> sharedLibrary)
        : _sharedLibrary(std::move(sharedLibrary))
        , _instance(std::move(instance))
    {
    }

    virtual ~IbExtensionProxy() = default;

protected:
    std::shared_ptr<IIbExtension> _sharedLibrary;// must live longer than instance
    std::unique_ptr<InterfaceType> _instance;
};

}//end namespace extensions
}//end namespace ib
