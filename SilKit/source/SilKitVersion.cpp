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

#include "SilKitVersion.hpp"
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
