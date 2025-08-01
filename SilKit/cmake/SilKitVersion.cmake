# SPDX-FileCopyrightText: 2024-025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# SIL Kit Versioning:
# * Major and minor release number is configured here. The patch number should not be changed here; it will be set by 
#   the Jenkins workflow to the master branch's build number for packaging (cmake -SILKIT_BUILD_NUMBER).
# * Major and minor release number, as well as the sprint number are encoded into Version.hpp and compiled into the library,
#   they will be accessible from public headers.
macro(configure_silkit_version project_name)
    set(SILKIT_VERSION_MAJOR 5)
    set(SILKIT_VERSION_MINOR 0)
    set(SILKIT_VERSION_PATCH 1)
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
