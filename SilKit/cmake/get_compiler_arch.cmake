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


function(get_linux_distro outVers)
    execute_process(COMMAND /usr/bin/lsb_release -s -i
        TIMEOUT 1
        OUTPUT_VARIABLE linux_id
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND /usr/bin/lsb_release -s -r
        TIMEOUT 1
        OUTPUT_VARIABLE linux_release
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${outVers} "${linux_id}-${linux_release}" PARENT_SCOPE)
endfunction()

function(get_ubuntu_version outVers)
    file(READ /etc/issue issue)
    string(REGEX REPLACE "^Ubuntu.*([0-9][0-9]+\.[0-9][0-9]).*" "\\1" issue "${issue}")
    set(${outVers} "${issue}" PARENT_SCOPE)
endfunction()

function(get_compiler_arch outComp outArch outPlatform  )
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
        set(SYSTEM_TAG "Win${SYSTEM_BITNESS}")
        set(${outPlatform} "Win${SYSTEM_BITNESS}" PARENT_SCOPE)
        if("${SYSTEM_BITNESS}" STREQUAL "64")
            set(SYSTEM_TAG "x64")
        endif()
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(${outPlatform} "Linux" PARENT_SCOPE)
        set(SYSTEM_TAG "x86")
        if(SYSTEM_BITNESS STREQUAL "64")
            set(SYSTEM_TAG "x64") #XXX should be amd64 or x86_64
        endif()
    else()
        set(SYSTEM_TAG "${CMAKE_SYSTEM_NAME}}")
    endif()
    set(${outArch} "${SYSTEM_TAG}" PARENT_SCOPE)

    # get toolset id + version
    set(_tool_tag "UNKNOWN")
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
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
    message(STATUS "Build target is: ${SYSTEM_TAG}-${_tool_tag}")
    set(${outComp} "${_tool_tag}" PARENT_SCOPE)
    set(${outArch} "${SYSTEM_TAG}" PARENT_SCOPE)
endfunction()
