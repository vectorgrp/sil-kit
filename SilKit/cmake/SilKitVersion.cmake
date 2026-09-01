# SPDX-FileCopyrightText: 2024-025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SIL Kit Versioning:
# * Major, minor and patch release number are configured here. This is the source of truth: the generated public header
#   SilKit/include/silkit/capi/SilKitVersionMacros.h is produced from these numbers and compiled into the library, so
#   they are accessible from public headers at runtime.
# * Do not edit the numbers below by hand. Run the sil-kit-generate-version tool, which keeps this file, the generated
#   header and the changelog in sync. See docs/development/release.md.
# * SILKIT_BUILD_NUMBER is a build-time override (cmake -DSILKIT_BUILD_NUMBER=N), not source tree state. It defaults to
#   0 and is passed to the sources as a compile definition; the generated header only carries the fallback.
macro(configure_silkit_version project_name)
    set(SILKIT_VERSION_MAJOR 5)
    set(SILKIT_VERSION_MINOR 0)
    set(SILKIT_VERSION_PATCH 8)
    set(SILKIT_BUILD_NUMBER 0 CACHE STRING "The build number")
    set(SILKIT_VERSION_SUFFIX "")

    set(VERSION_STRING "${SILKIT_VERSION_MAJOR}.${SILKIT_VERSION_MINOR}.${SILKIT_VERSION_PATCH}")
    if (SILKIT_VERSION_SUFFIX)
        set(VERSION_STRING "${VERSION_STRING}-${SILKIT_VERSION_SUFFIX}")
    endif()

    set(${project_name}_VERSION_MAJOR ${SILKIT_VERSION_MAJOR})
    set(${project_name}_VERSION_MINOR ${SILKIT_VERSION_MINOR})
    set(${project_name}_VERSION_PATCH ${SILKIT_VERSION_PATCH})
    set(${project_name}_VERSION_TWEAK ${SILKIT_BUILD_NUMBER})
    set(${project_name}_VERSION ${VERSION_STRING})

    set(PROJECT_VERSION_MAJOR ${SILKIT_VERSION_MAJOR})
    set(PROJECT_VERSION_MINOR ${SILKIT_VERSION_MINOR})
    set(PROJECT_VERSION_PATCH ${SILKIT_VERSION_PATCH})
    set(PROJECT_VERSION ${VERSION_STRING})
endmacro()
