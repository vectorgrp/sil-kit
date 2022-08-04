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
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//! \brief This release's major version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_Major_t)(uint32_t* outVersionMajor);

//! \brief This release's minor version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Minor(uint32_t* outVersionMinor);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_Minor_t)(uint32_t* outVersionMinor);

//! \brief This release's patch version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Patch(uint32_t* outVersionPatch);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_Patch_t)(uint32_t* outVersionPatch);

//! \brief Retrieve this release's build number on the master branch
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_BuildNumber_t)(uint32_t* outVersionBuildNumber);

//! \brief Retrieve the API version identifier "<Major>.<Minor>.<Patch>" of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_String(const char** outVersionString);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_String_t)(const char** outVersionString);

//! \brief Retrieve additional version information of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_VersionSuffix(const char** outVersionVersionSuffix);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_VersionSuffix_t)(const char** outVersionVersionSuffix);

//! \brief Retrieve the full git hash of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_GitHash(const char** outVersionGitHash);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Version_GitHash_t)(const char** outVersionGitHash);

SILKIT_END_DECLS

#pragma pack(pop)
