// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include "SilKitExtensionUtils.hpp"

#if !defined(SILKIT_EXTENSION_OS)
#   define SILKIT_EXTENSION_OS "UNKNOWN"
#endif

namespace SilKit { 


const char* BuildinfoSystem()
{
    return SILKIT_EXTENSION_OS;
}


}//end namespace SilKit
