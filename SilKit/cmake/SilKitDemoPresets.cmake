# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Generate and install a CMakePresets.json for building the demos against the pre-built SIL Kit package.
# The presets only offer the architecture of the package, so that an IDE (e.g. Visual Studio, "Open Folder")
# cannot pick a build type that does not match the pre-built SIL Kit.
function(silkit_install_demo_presets)
    cmake_parse_arguments(ARG "" "DESTINATION;COMPONENT" "" ${ARGN})

    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SILKIT_DEMOS_PRESET_PLATFORM "x64")
        else()
            set(SILKIT_DEMOS_PRESET_PLATFORM "x86")
        endif()
        set(SILKIT_DEMOS_PRESET_PREFIX "${SILKIT_DEMOS_PRESET_PLATFORM}-")
        set(SILKIT_DEMOS_PRESET_GENERATOR "Ninja")
        set(SILKIT_DEMOS_PRESET_HOST_OS "Windows")
        set(SILKIT_DEMOS_PRESET_ARCHITECTURE "
            \"architecture\": {
                \"value\": \"${SILKIT_DEMOS_PRESET_PLATFORM}\",
                \"strategy\": \"external\"
            },")
    else()
        set(SILKIT_DEMOS_PRESET_PLATFORM "${CMAKE_SYSTEM_NAME}")
        set(SILKIT_DEMOS_PRESET_PREFIX "")
        set(SILKIT_DEMOS_PRESET_GENERATOR "Unix Makefiles")
        set(SILKIT_DEMOS_PRESET_HOST_OS "${CMAKE_SYSTEM_NAME}")
        set(SILKIT_DEMOS_PRESET_ARCHITECTURE "")
    endif()

    set(_presetsFile "${CMAKE_CURRENT_BINARY_DIR}/SilKit-Demos/CMakePresets.json")
    configure_file(
        ${PROJECT_SOURCE_DIR}/Demos/CMakePresets.json.in
        ${_presetsFile}
        @ONLY
    )
    install(
        FILES ${_presetsFile}
        DESTINATION ${ARG_DESTINATION}
        COMPONENT ${ARG_COMPONENT}
    )
endfunction()
