# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

function(silkit_validate_preset_toolset)
    # When building using a preset, the variable SILKIT_REQUIRED_MSVC_TOOLSET_VERSION
    # is set. Validate that the user properly set vcvars corresponding to the preset.
    if(NOT SILKIT_REQUIRED_MSVC_TOOLSET_VERSION)
        return()
    endif()

    #accept any of "v140","v14.0","14.0"
    string(REGEX REPLACE "v?([0-9]+.?[0-9]*)" "\\1" _toolset "${SILKIT_REQUIRED_MSVC_TOOLSET_VERSION}")
    if(NOT MSVC_TOOLSET_VERSION MATCHES ${_toolset})
        message(FATAL_ERROR "SIL Kit: the cmake preset specifies MSVC toolset ${_toolset},"
                " but the currently selected MSVC_TOOLSET_VERSION is ${MSVC_TOOLSET_VERSION}."
                " Please source the appropriate vcvarsall.bat version for this build preset."
                " For example, `vcvarsall.bat -vcvars_ver=14.0 ...`.")
    endif()

endfunction()

function(get_uname outName outMachine)
    find_program(unameBin uname
        PATHS /bin /usr/bin /usr/local/bin
    )
    execute_process(COMMAND ${unameBin} -s
        TIMEOUT 1
        OUTPUT_VARIABLE unameName
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND ${unameBin} -m
        TIMEOUT 1
        OUTPUT_VARIABLE unameMachine
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${outName} "${unameName}" PARENT_SCOPE)
    set(${outMachine} "${unameMachine}" PARENT_SCOPE)
endfunction()

function(get_linux_distro outDistroName outDistroVersion)
    find_program(lsb_release lsb_release
        PATHS /bin /usr/bin /usr/local/bin
    )
    execute_process(COMMAND ${lsb_release} -s -i
        TIMEOUT 1
        OUTPUT_VARIABLE _linux_id
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND ${lsb_release} -s -r
        TIMEOUT 1
        OUTPUT_VARIABLE _linux_release
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    #sanitize output, strip spaces and return lower case
    if(_linux_id AND _linux_release)
        string(TOLOWER "${_linux_id}" _linux_id)
        string(REGEX REPLACE [^a-zA-Z0-9_-] "" _linux_id "${_linux_id}")
        string(REGEX REPLACE [^a-zA-Z0-9._-] "" _linux_release "${_linux_release}")
        set(${outDistroName} "${_linux_id}" PARENT_SCOPE)
        set(${outDistroVersion} "${_linux_release}" PARENT_SCOPE)
    endif()
endfunction()

function(get_compiler_arch outCompiler outArch outPlatform  )
    # compute bitness

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(SYSTEM_BITNESS "64")
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(SYSTEM_BITNESS "32")
    else()
        message(FATAL_ERROR "Unsupported pointer size: ${CMAKE_SIZEOF_VOID_P}")
    endif()

    message(STATUS "SIL Kit using ${SYSTEM_BITNESS}-bit build")
    message(STATUS "SIL Kit using compiler version ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

    # compute platform and architecture

    set(_platform "${CMAKE_SYSTEM_NAME}")
    set(_system_arch "UNKNOWN")

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(_platform "Win")

        # building on (or for) Windows-on-ARM will lead to invalid architecture values

        if("${SYSTEM_BITNESS}" STREQUAL "64")
            set(_system_arch "x86_64")
        else()
            set(_system_arch "x86")
        endif()
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "QNX")
        set(_system_arch "${SILKIT_TARGET_ARCHITECTURE}")
    elseif(UNIX)
        # generic unix has uname
        get_uname(_un_name _un_machine)
        set(_platform "${_un_name}")
        set(_system_arch "${_un_machine}")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        # on linux the platform tag is used for the distribution name and version
        get_linux_distro(_distro_name _distro_version)
        if( DEFINED _distro_name AND DEFINED _distro_version)
            set(_platform "${_distro_name}-${_distro_version}")
        endif()
    endif()

    # compute compiler name

    set(_tool_tag "UNKNOWN")

    if(MINGW)
        # currently any mingw compiler uses the tag MinGW for the compiler
        set(_tool_tag "MinGW")
    else()
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(_tool_tag "gcc")
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(_tool_tag "clang")
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            silkit_validate_preset_toolset()

            if("${MSVC_TOOLSET_VERSION}" STREQUAL "140")
                set(_tool_tag "VS2015")
            elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "141")
                set(_tool_tag "VS2017")
            elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "142")
                set(_tool_tag "VS2019")
            elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "143")
                set(_tool_tag "VS2022")
            elseif("${MSVC_TOOLSET_VERSION}" STREQUAL "144")
                set(_tool_tag "VS2022")
            elseif("${CMAKE_GENERATOR}" MATCHES "Visual Studio ")
                # fallback for VS generators
                string(REGEX REPLACE "Visual Studio [0-9]+ ([0-9]+) *.*" "\\1" vers "${CMAKE_GENERATOR}")
                set(_tool_tag "VS${vers}")
            else()
                # fallback for 'unhandled' MSVC toolset versions
                set(_tool_tag "MSVC${MSVC_TOOLSET_VERSION}")
            endif()
        else()
            # generic fallback
            set(_tool_tag "${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()

    # set output variables

    set(${outPlatform} "${_platform}" PARENT_SCOPE)
    set(${outCompiler} "${_tool_tag}" PARENT_SCOPE)
    set(${outArch} "${_system_arch}" PARENT_SCOPE)
endfunction()
