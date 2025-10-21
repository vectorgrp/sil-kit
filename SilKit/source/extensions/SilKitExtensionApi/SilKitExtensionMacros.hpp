// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "SilKitExtensionABI.h"
#include "SilKitExtensionUtils.hpp"

#define STRFY(x) #x

//! \brief Return build info array for the current platform
#define SILKIT_MAKE_BUILDINFOS() \
    { \
        SilKit::BuildinfoCPlusPlus(), SilKit::BuildinfoCompiler(), SilKit::BuildinfoMultithread(), \
            SilKit::BuildinfoDebug() \
    }

//! Declare extern C entry points for a extension and instantiate the
//  C++ interface.
#define SILKIT_DECLARE_EXTENSION(CLASS_NAME, VENDOR_STR, VMAJOR, VMINOR, VPATCH) \
    extern "C" \
    { \
        const SilKitExtensionDescriptor_t silkit_extension_descriptor{VMAJOR, \
                                                                      VMINOR, \
                                                                      VPATCH, \
                                                                      STRFY(CLASS_NAME), \
                                                                      VENDOR_STR, \
                                                                      SilKit::BuildinfoSystem(), \
                                                                      SILKIT_MAKE_BUILDINFOS()}; \
    } \
    SILEXT_API SILEXT_EXTENSION_HANDLE SILEXT_CABI CreateExtension() \
    { \
        try \
        { \
            SilKit::ISilKitExtension* instance = new CLASS_NAME(); \
            return instance; \
        } \
        catch (...) \
        { \
            return SILEXT_INVALID_HANDLE; \
        } \
    } \
    SILEXT_API void SILEXT_CABI ReleaseExtension(SILEXT_EXTENSION_HANDLE extension) \
    { \
        auto* instance = static_cast<CLASS_NAME*>(extension); \
        delete instance; \
    }
