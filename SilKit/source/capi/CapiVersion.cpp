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

extern "C"
{
    
SilKit_ReturnCode SilKit_Version_Major(uint32_t* outVersionMajor)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMajor);
    CAPI_ENTER 
    { 
        *outVersionMajor = SilKit::Version::Major();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_Minor(uint32_t* outVersionMinor)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionMinor);
    CAPI_ENTER 
    { 
        *outVersionMinor = SilKit::Version::Minor();
        return SilKit_ReturnCode_SUCCESS; 
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_Patch(uint32_t* outVersionPatch)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionPatch);
    CAPI_ENTER 
    { 
        *outVersionPatch = SilKit::Version::Patch();
        return SilKit_ReturnCode_SUCCESS; 
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionBuildNumber);
    CAPI_ENTER 
    { 
        *outVersionBuildNumber = SilKit::Version::BuildNumber();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_String(const char** outVersionString)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionString);
    CAPI_ENTER
    {
        *outVersionString = SilKit::Version::String();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_VersionSuffix(const char** outVersionSuffix)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionSuffix);
    CAPI_ENTER
    {
        *outVersionSuffix = SilKit::Version::VersionSuffix();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_Version_GitHash(const char** outVersionGitHash)
{
    ASSERT_VALID_OUT_PARAMETER(outVersionGitHash);
    CAPI_ENTER
    {
        *outVersionGitHash = SilKit::Version::GitHash();
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} //extern "C"
