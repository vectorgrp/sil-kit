// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

/*! SIL Kit Extension Implementations
 * Use the following SilKitExtensionBase class and to implement
 * shared libraries that act as SIL Kit extensions.
 */

#include <cstdint>
#include "SilKitExtensionABI.h"
#include "ISilKitExtension.hpp"

namespace SilKit {

//! \brief For convenience, implements an extension base with nice
//         accessors to the extension descriptor.
class SilKitExtensionBase : public ISilKitExtension
{
public:
    //! \brief The Ctor relies on the externally defined
    //         silkit_extension_descriptor from the implementation of the
    //         extension library.
    SilKitExtensionBase()
        : _descriptor(silkit_extension_descriptor)
    {
    }

    const char* GetExtensionName() const
    {
        return _descriptor.extension_name;
    }

    const char* GetVendorName() const
    {
        return _descriptor.vendor_name;
    }

    void GetVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) const
    {
        major = _descriptor.silkit_version_major;
        minor = _descriptor.silkit_version_minor;
        patch = _descriptor.silkit_version_patch;
    }

private:
    const SilKitExtensionDescriptor_t& _descriptor;
};


} //end namespace SilKit
