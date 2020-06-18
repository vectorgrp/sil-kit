// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

namespace ib { namespace extensions {

//forwards
class IIbExtension;

//The intention of using the proxy is to remove the deleters and other
//implementation details from the return value type of an Extension factory.
// This is basically a "Pointer to Implementation" idiom to hide the custom destructors,
// and to ensure that the destructors are run outside of the actual extension's
// memory segments.
// Usage:
// see CreateIbRegistry() and CreateMdf4tracing()

template<typename InterfaceType>
class IbExtensionProxy
    : public InterfaceType
{
public:
    using CustomDeleterT = std::function<void(InterfaceType*)>;
    using unique_ptr = std::unique_ptr<InterfaceType, CustomDeleterT>;

    IbExtensionProxy() = delete;
    IbExtensionProxy(IbExtensionProxy&&) = delete;

    IbExtensionProxy(unique_ptr instance,
        std::shared_ptr<IIbExtension> sharedLibrary)
        : _sharedLibrary(std::move(sharedLibrary))
        , _instance(std::move(instance))
    {
    }

    virtual ~IbExtensionProxy() = default;

protected:
    std::shared_ptr<IIbExtension> _sharedLibrary;// must live longer than instance
    unique_ptr _instance;
};

}//end namespace extensions
}//end namespace ib
