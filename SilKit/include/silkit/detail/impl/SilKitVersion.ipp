// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include "silkit/capi/Version.h"

#include "ThrowOnError.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Version {

auto Major() -> uint32_t
{
    uint32_t result{};
    const auto returnCode = SilKit_Version_Major(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto Minor() -> uint32_t
{
    uint32_t result{};
    const auto returnCode = SilKit_Version_Minor(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto Patch() -> uint32_t
{
    uint32_t result{};
    const auto returnCode = SilKit_Version_Patch(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto BuildNumber() -> uint32_t
{
    uint32_t result{};
    const auto returnCode = SilKit_Version_BuildNumber(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto String() -> const char*
{
    const char* result{nullptr};
    const auto returnCode = SilKit_Version_String(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto VersionSuffix() -> const char*
{
    const char* result{nullptr};
    const auto returnCode = SilKit_Version_VersionSuffix(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

auto GitHash() -> const char*
{
    const char* result{nullptr};
    const auto returnCode = SilKit_Version_GitHash(&result);
    Impl::ThrowOnError(returnCode);
    return result;
}

} // namespace Version
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Version {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Major;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Minor;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Patch;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::BuildNumber;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::String;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::VersionSuffix;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::GitHash;
} // namespace Version
} // namespace SilKit
