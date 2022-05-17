// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

#include "IbMacros.hpp"

namespace ib {
namespace version {

//! \brief This release's major version number
auto IntegrationBusAPI Major() -> uint32_t;
//! \brief This release's minor version number
auto IntegrationBusAPI Minor() -> uint32_t;
//! \brief This release's patch version number
auto IntegrationBusAPI Patch() -> uint32_t;
//! \brief Retrieve this release's build number on the master branch 
auto IntegrationBusAPI BuildNumber() -> uint32_t;
//! \brief Retrieve the API version identifier "<Major>.<Minor>.<Patch>" of this release
auto IntegrationBusAPI String() -> const char*;
//! \brief Retrieve additional version information of this release
auto IntegrationBusAPI VersionSuffix() -> const char*;
//! \brief Retrieve the full git hash of this release
auto IntegrationBusAPI GitHash() -> const char*;

} // namespace version
} // namespace ib
