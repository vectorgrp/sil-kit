# SPDX-FileCopyrightText: 2024-025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SIL Kit Versioning:
# * Major, minor and patch are read from SilKit/include/silkit/capi/SilKitVersionMacros.h, the single source of truth.
#   Bump with SilKit/ci/bump_version.py, see docs/development/release.md.
# * SILKIT_BUILD_NUMBER, SILKIT_BUILD_GIT_HASH and SILKIT_VERSION_SUFFIX describe a build, not the source tree. They are
#   passed to the sources as compile definitions and override the fallbacks in the header:
#     cmake -DSILKIT_BUILD_GIT_HASH=<hash> -DSILKIT_BUILD_NUMBER=N -DSILKIT_VERSION_SUFFIX=rc1
#   An empty SILKIT_BUILD_GIT_HASH defaults to 'git rev-parse HEAD'. The suffix also flows into VERSION_STRING below,
#   so CPack package names carry it too.
set(_SILKIT_VERSION_MACROS_H "${CMAKE_CURRENT_LIST_DIR}/../include/silkit/capi/SilKitVersionMacros.h")

macro(configure_silkit_version project_name)
    foreach(_component MAJOR MINOR PATCH)
        file(STRINGS "${_SILKIT_VERSION_MACROS_H}" _define REGEX "^#define SILKIT_VERSION_${_component} [0-9]+$")
        if(NOT _define)
            message(FATAL_ERROR "SIL Kit: no SILKIT_VERSION_${_component} in ${_SILKIT_VERSION_MACROS_H}")
        endif()
        string(REGEX REPLACE "^#define SILKIT_VERSION_${_component} " "" SILKIT_VERSION_${_component} "${_define}")
    endforeach()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_SILKIT_VERSION_MACROS_H}")

    set(SILKIT_BUILD_NUMBER 0 CACHE STRING "The build number")
    # Not named SILKIT_GIT_HASH: older build trees carry a stale INTERNAL cache
    # entry under that name, and set(... CACHE ...) would not overwrite it.
    set(SILKIT_BUILD_GIT_HASH "" CACHE STRING "Git hash of the built sources; empty uses 'git rev-parse HEAD'")
    set(SILKIT_VERSION_SUFFIX "" CACHE STRING "Pre-release suffix, e.g. rc1; empty for a normal build")

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
