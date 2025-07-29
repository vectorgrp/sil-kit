// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "SilKitVersionImpl.hpp"
#include "CapiImpl.hpp"


SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMajor);

    *outVersionMajor = SilKit::Version::MajorImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_Minor(uint32_t* outVersionMinor)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMinor);

    *outVersionMinor = SilKit::Version::MinorImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_Patch(uint32_t* outVersionPatch)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionPatch);

    *outVersionPatch = SilKit::Version::PatchImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionBuildNumber);

    *outVersionBuildNumber = SilKit::Version::BuildNumberImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_String(const char** outVersionString)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionString);

    *outVersionString = SilKit::Version::StringImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_VersionSuffix(const char** outVersionSuffix)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionSuffix);

    *outVersionSuffix = SilKit::Version::VersionSuffixImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Version_GitHash(const char** outVersionGitHash)
try
{
    ASSERT_VALID_OUT_PARAMETER(outVersionGitHash);

    *outVersionGitHash = SilKit::Version::GitHashImpl();
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
