# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
    #get arch
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(SYSTEM_BITNESS "64")
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(SYSTEM_BITNESS "32")
    else()
        message(FATAL_ERROR "Bitness is not supported: \"${CMAKE_SIZEOF_VOID_P}\"")
    endif()
    message(STATUS "SIL Kit using ${SYSTEM_BITNESS}-Bit build")

    #get OS
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(_platform "Win")

        set(_system_arch "x86")
        if("${SYSTEM_BITNESS}" STREQUAL "64")
            set(_system_arch "x86_64")
        endif()
    elseif(UNIX)
        # generic unix has uname
        get_uname(_un_name _un_machine)
        set(_platform "${_un_name}")
        set(_system_arch "${_un_machine}")
    else()
        set(_system_arch "${CMAKE_SYSTEM_NAME}")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        get_linux_distro(_distro_name _distro_version)
        if( DEFINED _distro_name AND DEFINED _distro_version)
            set(_platform "${_distro_name}-${_distro_version}")
        endif()
    endif()

    # get toolset/compiler id + version
    set(_tool_tag "UNKNOWN")
    if(UNIX)
        set(_id "${CMAKE_CXX_COMPILER_ID}")
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(_id "gcc")
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(_id "clang")
        endif()
        message(STATUS "Using compiler version: ${_id}-${CMAKE_CXX_COMPILER_VERSION}")
        set(_tool_tag ${_id})
    elseif(MINGW)
        set(_tool_tag "MinGW")
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        silkit_validate_preset_toolset()
        if(CMAKE_VS_PLATFORM_TOOLSET MATCHES "v140")
            set(_tool_tag "VS2015")
        elseif(CMAKE_VS_PLATFORM_TOOLSET MATCHES "v141")
            set(_tool_tag "VS2017")
        elseif(CMAKE_VS_PLATFORM_TOOLSET MATCHES "v142")
            set(_tool_tag "VS2019")
        elseif(MSVC_TOOLSET_VERSION MATCHES "142")
            set(_tool_tag "VS2019")
        elseif(MSVC_TOOLSET_VERSION MATCHES "141")
            set(_tool_tag "VS2017")
        elseif(MSVC_TOOLSET_VERSION MATCHES "140")
            set(_tool_tag "VS2015")
        else()
            string(REGEX REPLACE "Visual Studio [0-9]+ ([0-9]+) *.*" "\\1" vers "${CMAKE_GENERATOR}")
            set(_tool_tag "VS${vers}")
        endif()
    endif()
    #return  triple
    set(${outPlatform} "${_platform}" PARENT_SCOPE)
    set(${outCompiler} "${_tool_tag}" PARENT_SCOPE)
    set(${outArch} "${_system_arch}" PARENT_SCOPE)
endfunction()
