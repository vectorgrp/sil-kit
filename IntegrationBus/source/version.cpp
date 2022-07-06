// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "version.hpp"
#include "version_macros.hpp"

namespace SilKit {
namespace Version {
        
auto SilKitAPI Major() -> uint32_t
{
    return SILKIT_VERSION_MAJOR;
}
    
auto SilKitAPI Minor() -> uint32_t
{
    return SILKIT_VERSION_MINOR;
}
    
auto SilKitAPI Patch() -> uint32_t
{
    return SILKIT_VERSION_PATCH;
}

auto SilKitAPI BuildNumber() -> uint32_t
{
    return SILKIT_BUILD_NUMBER;
}

auto SilKitAPI String() -> const char*
{
    return SILKIT_VERSION_STRING;
}

auto SilKitAPI VersionSuffix() -> const char*
{
    return SILKIT_VERSION_SUFFIX;
}

auto SilKitAPI GitHash() -> const char*
{
    return SILKIT_GIT_HASH;
}

} // namespace Version
} // namespace SilKit
