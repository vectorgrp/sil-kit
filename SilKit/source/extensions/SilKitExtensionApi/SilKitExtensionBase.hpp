/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

/*! SIL Kit Extension Implementations
 * Use the following SilKitExtensionBase class and to implement
 * shared libraries that act as SILKIT extensions.
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
        :_descriptor(silkit_extension_descriptor)
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

    void GetVersion(uint32_t& major,
            uint32_t& minor, uint32_t& patch) const
    {
        major = _descriptor.silkit_version_major;
        minor = _descriptor.silkit_version_minor;
        patch = _descriptor.silkit_version_patch;
    }

private:

    const SilKitExtensionDescriptor_t&  _descriptor;
};



}//end namespace SilKit
