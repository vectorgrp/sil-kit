// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Source of truth for the SIL Kit version. SilKit/cmake/SilKitVersion.cmake
// reads the three numbers below. Bump with SilKit/ci/bump_version.py, see
// docs/development/release.md. Keep this file C11 and resource compiler
// compatible.

#pragma once

#define SILKIT_VERSION_MAJOR 5
#define SILKIT_VERSION_MINOR 0
#define SILKIT_VERSION_PATCH 8

// Build properties. CMake overrides these, the values here are fallbacks:
//   -DSILKIT_BUILD_NUMBER=N
//   -DSILKIT_BUILD_GIT_HASH=<hash>  (default: git rev-parse HEAD)
//   -DSILKIT_VERSION_SUFFIX=rc1     (also appended to SILKIT_VERSION_STRING)
#ifndef SILKIT_BUILD_NUMBER
#define SILKIT_BUILD_NUMBER 0
#endif

#ifndef SILKIT_GIT_HASH
#define SILKIT_GIT_HASH "UNKNOWN"
#endif

#ifndef SILKIT_VERSION_SUFFIX
#define SILKIT_VERSION_SUFFIX ""
#endif

#define SILKIT_VERSION_STRINGIFY_(x) #x
#define SILKIT_VERSION_STRINGIFY(x) SILKIT_VERSION_STRINGIFY_(x)

#ifndef SILKIT_VERSION_STRING
#define SILKIT_VERSION_STRING \
    SILKIT_VERSION_STRINGIFY(SILKIT_VERSION_MAJOR) \
    "." SILKIT_VERSION_STRINGIFY(SILKIT_VERSION_MINOR) "." SILKIT_VERSION_STRINGIFY(SILKIT_VERSION_PATCH)
#endif
