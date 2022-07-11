// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

#include "SilKitMacros.hpp"

namespace SilKit {
namespace Version {

//! \brief This release's major version number
auto SilKitAPI Major() -> uint32_t;
//! \brief This release's minor version number
auto SilKitAPI Minor() -> uint32_t;
//! \brief This release's patch version number
auto SilKitAPI Patch() -> uint32_t;
//! \brief Retrieve this release's build number on the master branch 
auto SilKitAPI BuildNumber() -> uint32_t;
//! \brief Retrieve the API version identifier "<Major>.<Minor>.<Patch>" of this release
auto SilKitAPI String() -> const char*;
//! \brief Retrieve additional version information of this release
auto SilKitAPI VersionSuffix() -> const char*;
//! \brief Retrieve the full git hash of this release
auto SilKitAPI GitHash() -> const char*;

} // namespace Version
} // namespace SilKit
