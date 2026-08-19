# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

################################################################################
# SBOM generation
################################################################################
# Software composition scanners cannot resolve this project's dependencies: they are git submodules
# carrying nothing but a gitlink, plus one vendored source amalgamation. The inventory is therefore
# declared in ThirdParty/third-party-components.json and rendered to SPDX 2.3 by
# SilKit/ci/generate_sbom.py.
#
# Targets:
#   silkit-sbom         (in ALL) writes ${CMAKE_BINARY_DIR}/sbom/ for the configuration being built
#   silkit-sbom-update  refreshes the canonical SilKit.spdx.json committed to the repository
#   silkit-sbom-check   verifies the committed SBOM and the metadata, as CI does

function(silkit_add_sbom)
    if(NOT SILKIT_BUILD_SBOM)
        return()
    endif()

    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(NOT Python3_Interpreter_FOUND)
        # Degrade rather than fail: MinGW, macOS and the cross-compilation presets must keep
        # building without a Python interpreter present.
        message(STATUS "SIL Kit - SBOM: no Python 3 interpreter found, skipping SBOM generation")
        return()
    endif()

    set(sbomScript "${PROJECT_SOURCE_DIR}/SilKit/ci/generate_sbom.py")
    set(sbomMetadata "${PROJECT_SOURCE_DIR}/ThirdParty/third-party-components.json")
    set(sbomOutput "${CMAKE_BINARY_DIR}/sbom/SilKit-${PROJECT_VERSION}.spdx.json")

    if(NOT EXISTS "${sbomScript}" OR NOT EXISTS "${sbomMetadata}")
        message(STATUS "SIL Kit - SBOM: generator or metadata missing, skipping SBOM generation")
        return()
    endif()

    set(sbomArgs --version "${PROJECT_VERSION}" --output "${sbomOutput}")

    # Record what was actually built, so the SBOM does not claim artifacts this configuration
    # never produced. The generator's defaults describe a full release.
    if(NOT SILKIT_BUILD_UTILITIES)
        list(APPEND sbomArgs --without-utilities)
    endif()
    if(NOT SILKIT_BUILD_DOCS)
        list(APPEND sbomArgs --without-docs)
    endif()
    if(NOT SILKIT_INSTALL_SOURCE)
        list(APPEND sbomArgs --without-source)
    endif()
    if(NOT SILKIT_BUILD_DASHBOARD)
        list(APPEND sbomArgs --without-dashboard)
    endif()
    if(SILKIT_USE_SYSTEM_LIBRARIES)
        list(APPEND sbomArgs --use-system-libraries)
    endif()

    # SILKIT_GIT_HASH is set by MakeVersionMacros.cmake. It is absent for packaged source trees,
    # in which case the generator falls back to the version tag.
    if(SILKIT_GIT_HASH AND NOT SILKIT_GIT_HASH STREQUAL "UNKNOWN")
        list(APPEND sbomArgs --git-hash "${SILKIT_GIT_HASH}")
    endif()

    add_custom_command(
        OUTPUT "${sbomOutput}"
        COMMAND "${Python3_EXECUTABLE}" "${sbomScript}" ${sbomArgs}
        DEPENDS "${sbomScript}" "${sbomMetadata}"
        COMMENT "Generating SPDX SBOM ${sbomOutput}"
        VERBATIM
    )

    add_custom_target(silkit-sbom ALL DEPENDS "${sbomOutput}")
    set_property(TARGET silkit-sbom PROPERTY FOLDER "Packaging")

    # Both of the following deliberately use the generator's defaults, which are the canonical
    # configuration of the SBOM committed to the repository. They must not inherit the flags of
    # the current build.
    add_custom_target(silkit-sbom-update
        COMMAND "${Python3_EXECUTABLE}" "${sbomScript}"
        COMMENT "Updating the canonical SilKit.spdx.json"
        VERBATIM
    )
    set_property(TARGET silkit-sbom-update PROPERTY FOLDER "Packaging")

    add_custom_target(silkit-sbom-check
        COMMAND "${Python3_EXECUTABLE}" "${sbomScript}" --check
        COMMENT "Checking the canonical SilKit.spdx.json against the source tree"
        VERBATIM
    )
    set_property(TARGET silkit-sbom-check PROPERTY FOLDER "Packaging")

    message(STATUS "SIL Kit - SBOM: ${sbomOutput}")
endfunction()
