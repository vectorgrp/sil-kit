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

function(silkit_enable_asan isOn)
    if(NOT isOn)
        return()
    endif()
    message(STATUS "SIL Kit -- Enabling Address Sanitizer")
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        #clang, gcc
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer )
        #ensures that -lasan is linked in as first library
        add_link_options(-fsanitize=address)
    endif()
endfunction()

function(silkit_enable_ubsan isOn)
    if(NOT isOn)
        return()
    endif()
    if(MSVC)
        message(STATUS "SIL Kit -- UBSAN not supported on MSVC")
    else()
        message(STATUS "SIL Kit -- Enabling Undefined Behavior Sanitizer")
        #clang, gcc
        add_compile_options(-fsanitize=undefined) 
        #ensures that -lasan is linked in as first library
        add_link_options(-fsanitize=undefined)
    endif()
endfunction()

function(silkit_enable_threadsan isOn)
    if(NOT isOn)
        return()
    endif()
    if(MSVC)
        message(STATUS "SIL Kit -- Thread Sanitizer not supported on MSVC")
    else()
        message(STATUS "SIL Kit -- Enabling Thread Sanitizer")
        #clang, gcc
        add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g3) 
        #ensures that -lasan is linked in as first library
        add_link_options(-fsanitize=thread -fno-omit-frame-pointer -g3)
    endif()
endfunction()

function(silkit_enable_coverage isOn)
    if(NOT isOn)
        return()
    endif()
    if(MSVC)
        message(STATUS "SIL Kit -- Coverage not supported on MSVC")
    else()
        message(STATUS "SIL Kit -- Enabling Coverage")
        #clang, gcc
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(-fprofile-arcs -ftest-coverage)
    endif()
endfunction()

# Clean warning defaults set in CMAKE_*_FLAGS by CMake < 3.15
function(silkit_replace_compileflags regexp value)
    foreach (flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        # Clean /W3 to avoid /W3 /W4 override warnings
        string(REGEX REPLACE "${regexp}" "${value}" ${flag_var} "${${flag_var}}")
        set(${flag_var} "${${flag_var}}" PARENT_SCOPE)
    endforeach()
endfunction()

# Remove warning levels which are injected by old CMake versions.
# We set the warning flags explicitly.
macro(silkit_clean_default_compileflags)
    if(MSVC)
        silkit_replace_compileflags("(/W[1|2|3|4]|/w)" "")
        add_compile_options("/MP")
        if(MSVC_VERSION EQUAL 1900)
            #disable: "decorated name length exceeded", we have long templates.
            add_compile_options("/wd4503")
        endif()
    endif()
endmacro()

function(silkit_target_clean_compileflags target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "silkit_target_clean_compileflags: the target ${target} does not exist")
    endif()

    if(MSVC)
        get_target_property(_compile_options ${target} COMPILE_OPTIONS)
        list(TRANSFORM _compile_options REPLACE "/W[1|2|3|4]" "")
        set_target_properties(${target} PROPERTIES COMPILE_OPTIONS "${_compile_options}")
    endif()
endfunction()
function(silkit_enable_warnings isOn)
    # Conditionally treat warnings as errors
    set(_warnAsError "")
    if (${isOn})
        # MSVC_VERSION 1900 is VS2015 aka v140
        # we do not enable warnings as errors on VS2015 and below!
        if(MSVC_VERSION GREATER 1900)
            message(STATUS "SIL Kit: enabling warnings as errors!")
            set(_warnAsError "/WX")
        elseif(UNIX OR MINGW)
            #assume compiler is clang or gcc
            message(STATUS "SIL Kit: enabling warnings as errors!")
            set(_warnAsError "-Werror")
        endif()
    endif()

    # actual warning flags per platform/compiler
    if(MSVC)
        silkit_clean_default_compileflags()
        set(_flags 
            /W4
            /wd4100 # disable unreferenced formal parameter
            ${_warnAsError}
            )
        # Warning levels for VS2017
        if(MSVC_VERSION GREATER 1910 AND MSVC_VERSION LESS 1920)
            set(_flags  ${_flags}
                # libfmt has a deliberate 'integral constant overflow', c.f. https://github.com/fmtlib/fmt/issues/3163
                /wd4307 # integral constant overflow
                /wd4244 # possible loss of data after conversion
                /wd4389 # too many bogus signed/unsigned warnings in VS2017 Win32
            )
        endif()
    elseif(MINGW)
        set(_flags
            -pedantic
            -Wall
            -Wextra
            -Wcast-align
            -Wpacked
            -Wno-implicit-fallthrough
            -Wno-error=dangling-reference

            -Wno-shadow                 # Appears in ThirdParty/spdlog/include/spdlog/common.h:214:9
            -Wno-format                 # MinGW-gcc does not recognize %zu
            -Wno-unused-parameter       # follow up of the %zu format bug, a lot of unused parameters
            -Wstrict-overflow=1         # > 1 fails in fmt-6.1.0
            ${_warnAsError}
            )
    else()
        set(_flags
            -pedantic
            -Wall
            -Wextra
            -Wcast-align
            -Wpacked
            -Wno-implicit-fallthrough
            -Wformat=2

            -Wno-shadow                 # Appears in ThirdParty/spdlog/include/spdlog/common.h:214:9
            -Wno-format-nonliteral      # Warning in fmt-6.1.0/include/fmt/chrono.h:392:48
            -Wstrict-overflow=1         # > 1 fails in fmt-6.1.0
            ${_warnAsError}
            )

    endif()

    # Suppress flaky dangling-reference warning
    # see https://gcc.gnu.org/git/gitweb.cgi?p=gcc.git;h=6b927b1297e66e26e62e722bf15c921dcbbd25b9
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13")
            set(_flags ${_flags}
                -Wno-dangling-reference
                )
        endif()
    endif()

    add_compile_options(${_flags})
endfunction()

macro(silkit_check_reproducible)
    if(NOT DEFINED ENV{SOURCE_DATE_EPOCH})
        message(STATUS "SIL Kit - Reproducible build: SOURCE_DATE_EPOCH is not set!"
           " Set it to 'git log -1 --format=%ct -r origin/main'")
    else()
        message(STATUS "SIL Kit - Reproducible build: SOURCE_DATE_EPOCH=$ENV{SOURCE_DATE_EPOCH}")
    endif()
    if(NOT "$ENV{LC_ALL}" STREQUAL  "C")
        message(STATUS "SIL Kit - Reproducible build: LC_ALL is not set to 'C': '$ENV{LC_ALL}'")
    else()
        message(STATUS "SIL Kit - Reproducible build: LC_ALL=C")
    endif()
    if(NOT "$ENV{TZ}" STREQUAL  "UTC")
        message(STATUS "SIL Kit - Reproducible build: TZ is not set to 'UTC': '$ENV{TZ}'")
    else()
        message(STATUS "SIL Kit - Reproducible build: TZ=UTC")
    endif()

endmacro()


include(CheckCXXSourceCompiles)
macro(silkit_add_libs_for_atomic64)
# armv7 gcc might need -latomic for 64bit atomic operations (__atomic_fetch_add_8)
    check_cxx_source_compiles("
#include <atomic>
#include <cstdint>
int main() {
    std::atomic<int64_t> a{0};
    int64_t b{10};
    a =  a + b;
    return a;
 }
" HAVE_ATOMIC_64BIT)
    if(NOT APPLE AND NOT HAVE_ATOMIC_64BIT)
        link_libraries(-latomic)
    endif()
endmacro()
