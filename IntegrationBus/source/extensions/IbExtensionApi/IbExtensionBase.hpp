// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

/*! IB Extension Implementations
 * Use the following IbExtensionBase class and to implement
 * shared libraries that act as VIB extensions.
 */

#include <cstdint>
#include "IbExtensionABI.h"
#include "IIbExtension.hpp"

namespace ib { namespace extensions {

//! \brief For convenience, implements an extension base with nice 
//         accessors to the extension descriptor.
class IbExtensionBase : public IIbExtension
{
public:

    //! \brief The Ctor relies on the externally defined
    //         vib_extension_descriptor from the implementation of the 
    //         extension library.
    IbExtensionBase()
        :_descriptor(vib_extension_descriptor)
    {
    }

    const char*GetExtensionName() const
    {
        return _descriptor.extension_name;
    }

    const char* GetVendorName() const
    {
        return _descriptor.vendor_name;
    }

    void GetVersion(uint32_t& major,
            uint32_t& minor, uint32_t& patch) const
    {
        major = _descriptor.vib_version_major;
        minor = _descriptor.vib_version_minor;
        patch = _descriptor.vib_version_patch;
    }

private:

    const IbExtensionDescriptor_t&  _descriptor;
};


}//end namespace extensions
}//end namespace ib
