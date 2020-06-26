// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <memory>

namespace ib { namespace extensions {

//forwards of private interfaces
class IIbExtension;

//! \brief ExtensionHandle is a smart pointer that bundles a loaded shared
//         library and an instance of the library's extension interface.
// The ExtensionHandle keeps the shared library loaded as long as the instance
// is in use. This prevents the shared library from being unloaded while code or
// data of the ExtensionInterfaceT is still in use.
// Use Get() or operator->() to access a pointer to the extension interface.

template<typename ExtensionInterfaceT>
class ExtensionHandle
{
public:
    ExtensionHandle(const ExtensionHandle&) = delete;
    ExtensionHandle operator=(const ExtensionHandle&) = delete;

    ExtensionHandle() = default;
    ExtensionHandle(ExtensionHandle&& otherHolder) = default;
    ExtensionHandle& operator=(ExtensionHandle&& otherHolder) = default;

    ExtensionHandle( std::shared_ptr<IIbExtension> sharedLibrary,
        std::unique_ptr<ExtensionInterfaceT> instance)
    : _sharedLibrary{std::move(sharedLibrary)} 
    , _instance{std::move(instance)}
    {
    }

    inline ExtensionInterfaceT* Get() const;
    inline ExtensionInterfaceT* operator->() const;

    inline bool operator==(const ExtensionHandle& other) const;
    inline bool operator!=(const ExtensionHandle& other) const;
    inline explicit operator bool() const;

private:
    // must be hold onto until _instance is destroyed:
    std::shared_ptr<IIbExtension> _sharedLibrary;
    std::unique_ptr<ExtensionInterfaceT> _instance;
};

//////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////

template<typename ExtensionInterfaceT>
inline ExtensionInterfaceT* ExtensionHandle<ExtensionInterfaceT>::Get() const
{
    return _instance.get();
}

template<typename ExtensionInterfaceT>
inline ExtensionInterfaceT* ExtensionHandle<ExtensionInterfaceT>::operator->() const
{
    return _instance.get();
}

template<typename T>
bool ExtensionHandle<T>::operator==(const ExtensionHandle<T>& other) const
{
    return _instance == other._instance
        && _sharedLibrary == other._sharedLibrary;
}

template<typename T>
bool ExtensionHandle<T>::operator!=(const ExtensionHandle<T>& other) const
{
    return  !(other == *this);
}

template<typename T>
ExtensionHandle<T>::operator bool() const
{
    return _instance.operator bool();
}

}//end namespace extensions
}//end namespace ib
