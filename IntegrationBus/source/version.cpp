// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "version.hpp"
#include "version_macros.hpp"

namespace ib {
namespace version {
        
auto IntegrationBusAPI Major() -> uint32_t
{
    return IB_VERSION_MAJOR;
}
    
auto IntegrationBusAPI Minor() -> uint32_t
{
    return IB_VERSION_MINOR;
}
    
auto IntegrationBusAPI Patch() -> uint32_t
{
    return IB_VERSION_PATCH;
}

auto IntegrationBusAPI BuildNumber() -> uint32_t
{
    return IB_BUILD_NUMBER;
}

auto IntegrationBusAPI String() -> const char*
{
    return IB_VERSION_STRING;
}

auto IntegrationBusAPI SprintNumber() -> uint32_t
{
    return IB_SPRINT_NUMBER;
}
    
auto IntegrationBusAPI SprintName() -> const char*
{
    return IB_SPRINT_NAME;
}
    
auto IntegrationBusAPI GitHash() -> const char*
{
    return IB_GIT_HASH;
}

} // namespace version
} // namespace ib
