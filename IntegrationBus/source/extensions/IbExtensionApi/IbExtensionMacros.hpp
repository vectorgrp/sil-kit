// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IbExtensionApi/IbExtensionABI.h"
#include "IbExtensionApi/IbExtensionUtils.hpp"

#define STRFY(x) #x

//! \brief Return build info array for the current platform
#define IB_MAKE_BUILDINFOS()\
    {\
        ib::extensions::BuildinfoCPlusPlus(),\
        ib::extensions::BuildinfoCompiler(),\
        ib::extensions::BuildinfoMultithread(),\
        ib::extensions::BuildinfoDebug()\
    }

//! Declare extern C entry points for a extension and instantiate the 
//  C++ interface.
#define IB_DECLARE_EXTENSION(CLASS_NAME, VENDOR_STR, VMAJOR, VMINOR, VPATCH) \
    extern "C" {\
        const IbExtensionDescriptor_t vib_extension_descriptor{\
            VMAJOR,\
            VMINOR,\
            VPATCH,\
            STRFY(CLASS_NAME),\
            VENDOR_STR,\
            ib::extensions::BuildinfoSystem(),\
            IB_MAKE_BUILDINFOS()\
        };\
    }\
    VIBE_API VIBE_EXTENSION_HANDLE VIBE_CABI CreateExtension()\
    {\
        try {\
            ib::extensions::IIbExtension* instance = new CLASS_NAME();\
            return instance;\
        } \
        catch(...) \
        { \
            return VIBE_INVALID_HANDLE;\
        }\
    }\
    VIBE_API void VIBE_CABI ReleaseExtension(VIBE_EXTENSION_HANDLE extension)\
    {\
        auto* instance = static_cast<CLASS_NAME *>(extension);\
        delete instance;\
    }

