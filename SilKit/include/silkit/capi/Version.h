// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//! \brief This release's major version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_Major_t)(uint32_t* outVersionMajor);

//! \brief This release's minor version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Minor(uint32_t* outVersionMinor);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_Minor_t)(uint32_t* outVersionMinor);

//! \brief This release's patch version number
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Patch(uint32_t* outVersionPatch);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_Patch_t)(uint32_t* outVersionPatch);

//! \brief Retrieve this release's build number on the master branch
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_BuildNumber(uint32_t* outVersionBuildNumber);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_BuildNumber_t)(uint32_t* outVersionBuildNumber);

//! \brief Retrieve the API version identifier "<Major>.<Minor>.<Patch>" of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_String(const char** outVersionString);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_String_t)(const char** outVersionString);

//! \brief Retrieve additional version information of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_VersionSuffix(const char** outVersionVersionSuffix);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_VersionSuffix_t)(const char** outVersionVersionSuffix);

//! \brief Retrieve the full git hash of this release
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_GitHash(const char** outVersionGitHash);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Version_GitHash_t)(const char** outVersionGitHash);

SILKIT_END_DECLS

#pragma pack(pop)
