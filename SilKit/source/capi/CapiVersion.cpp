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

#include "silkit/capi/SilKit.h"
#include "silkit/SilKitVersion.hpp"
#include "CapiImpl.hpp"

SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMajor);

    *outVersionMajor = SilKit::Version::Major();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_Minor(uint32_t* outVersionMinor)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMinor);

    *outVersionMinor = SilKit::Version::Minor();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_Patch(uint32_t* outVersionPatch)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionPatch);

    *outVersionPatch = SilKit::Version::Patch();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionBuildNumber);

    *outVersionBuildNumber = SilKit::Version::BuildNumber();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_String(const char** outVersionString)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionString);

    *outVersionString = SilKit::Version::String();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_VersionSuffix(const char** outVersionSuffix)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionSuffix);

    *outVersionSuffix = SilKit::Version::VersionSuffix();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE

SilKit_ReturnCode SilKitCALL SilKit_Version_GitHash(const char** outVersionGitHash)
CAPI_ENTER
{
    ASSERT_VALID_OUT_PARAMETER(outVersionGitHash);

    *outVersionGitHash = SilKit::Version::GitHash();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_LEAVE
