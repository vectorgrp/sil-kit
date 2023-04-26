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
