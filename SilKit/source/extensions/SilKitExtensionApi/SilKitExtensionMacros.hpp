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

#include "SilKitExtensionABI.h"
#include "SilKitExtensionUtils.hpp"

#define STRFY(x) #x

//! \brief Return build info array for the current platform
#define SILKIT_MAKE_BUILDINFOS()\
    {\
        SilKit::BuildinfoCPlusPlus(),\
        SilKit::BuildinfoCompiler(),\
        SilKit::BuildinfoMultithread(),\
        SilKit::BuildinfoDebug()\
    }

//! Declare extern C entry points for a extension and instantiate the 
//  C++ interface.
#define SILKIT_DECLARE_EXTENSION(CLASS_NAME, VENDOR_STR, VMAJOR, VMINOR, VPATCH) \
    extern "C" {\
        const SilKitExtensionDescriptor_t silkit_extension_descriptor{\
            VMAJOR,\
            VMINOR,\
            VPATCH,\
            STRFY(CLASS_NAME),\
            VENDOR_STR,\
            SilKit::BuildinfoSystem(),\
            SILKIT_MAKE_BUILDINFOS()\
        };\
    }\
    SILEXT_API SILEXT_EXTENSION_HANDLE SILEXT_CABI CreateExtension()\
    {\
        try {\
            SilKit::ISilKitExtension* instance = new CLASS_NAME();\
            return instance;\
        } \
        catch(...) \
        { \
            return SILEXT_INVALID_HANDLE;\
        }\
    }\
    SILEXT_API void SILEXT_CABI ReleaseExtension(SILEXT_EXTENSION_HANDLE extension)\
    {\
        auto* instance = static_cast<CLASS_NAME *>(extension);\
        delete instance;\
    }

