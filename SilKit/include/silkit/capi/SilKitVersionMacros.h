// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// GENERATED FILE - DO NOT EDIT BY HAND.
// Written by the sil-kit-generate-version tool from the version numbers in
// SilKit/cmake/SilKitVersion.cmake. See docs/development/release.md.

#pragma once

#define SILKIT_VERSION_MAJOR 5
#define SILKIT_VERSION_MINOR 0
#define SILKIT_VERSION_PATCH 8

// Everything below describes a build rather than the source tree, so CMake
// supplies it and the values here are only fallbacks:
//   -DSILKIT_BUILD_NUMBER=N         stamps a build number
//   -DSILKIT_BUILD_GIT_HASH=<hash>  the commit actually built; the fallback is
//                                   the commit that was HEAD when this file was
//                                   generated, i.e. the parent of the bump
//   -DSILKIT_VERSION_SUFFIX=rc1     marks a pre-release, which also changes
//                                   SILKIT_VERSION_STRING to "5.0.8-rc1"
#ifndef SILKIT_BUILD_NUMBER
#define SILKIT_BUILD_NUMBER 0
#endif

#ifndef SILKIT_GIT_HASH
#define SILKIT_GIT_HASH "b5fa2b126cd9538a0f5889bfffd99d2df2985528"
#endif

#ifndef SILKIT_VERSION_SUFFIX
#define SILKIT_VERSION_SUFFIX ""
#endif

#ifndef SILKIT_VERSION_STRING
#define SILKIT_VERSION_STRING "5.0.8"
#endif
