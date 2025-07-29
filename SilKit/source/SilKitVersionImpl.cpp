// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SilKitVersionImpl.hpp"

#include "version_macros.hpp"

namespace SilKit {
namespace Version {

auto MajorImpl() -> uint32_t
{
    return SILKIT_VERSION_MAJOR;
}

auto MinorImpl() -> uint32_t
{
    return SILKIT_VERSION_MINOR;
}

auto PatchImpl() -> uint32_t
{
    return SILKIT_VERSION_PATCH;
}

auto BuildNumberImpl() -> uint32_t
{
    return SILKIT_BUILD_NUMBER;
}

auto StringImpl() -> const char*
{
    return SILKIT_VERSION_STRING;
}

auto VersionSuffixImpl() -> const char*
{
    return SILKIT_VERSION_SUFFIX;
}

auto GitHashImpl() -> const char*
{
    return SILKIT_GIT_HASH;
}

} // namespace Version
} // namespace SilKit
