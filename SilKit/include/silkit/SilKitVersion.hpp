// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Version {

//! \brief This release's major version number
DETAIL_SILKIT_CPP_API auto Major() -> uint32_t;

//! \brief This release's minor version number
DETAIL_SILKIT_CPP_API auto Minor() -> uint32_t;

//! \brief This release's patch version number
DETAIL_SILKIT_CPP_API auto Patch() -> uint32_t;

//! \brief Retrieve this release's build number on the master branch
DETAIL_SILKIT_CPP_API auto BuildNumber() -> uint32_t;

//! \brief Retrieve the API version identifier "<Major>.<Minor>.<Patch>" of this release
DETAIL_SILKIT_CPP_API auto String() -> const char*;

//! \brief Retrieve additional version information of this release
DETAIL_SILKIT_CPP_API auto VersionSuffix() -> const char*;

//! \brief Retrieve the full git hash of this release
DETAIL_SILKIT_CPP_API auto GitHash() -> const char*;

} // namespace Version
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/SilKitVersion.ipp"
//! \endcond
