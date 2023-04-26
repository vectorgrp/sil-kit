/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
