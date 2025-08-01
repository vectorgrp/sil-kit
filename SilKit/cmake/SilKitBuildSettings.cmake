# SPDX-FileCopyrightText: 2022-2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT


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
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
		link_libraries(-lstdc++fs)
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

macro(silkit_enable_lto enableLto)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _lto_is_supported OUTPUT error)

    if ((NOT MINGW) AND ${enableLto} AND  _lto_is_supported)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

            find_program(LLD_EXECUTABLE NAMES lld ld.lld)
            if(LLD_EXECUTABLE)
                message("SIL Kit: Found LLD linker: ${LLD_EXECUTABLE}")
                # Test if clang can use LLD
                execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=lld --version 
                    RESULT_VARIABLE LLD_TEST_RESULT OUTPUT_QUIET ERROR_QUIET)
                if(LLD_TEST_RESULT EQUAL 0)
                    message("SIL Kit: Using LLD linker for LTO")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
                    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
                else()
                    message(WARNING "SIL Kit: LLD found but not compatible with this Clang version")
                endif()
            else()
                # Fallback to Gold linker
                find_program(GOLD_EXECUTABLE NAMES gold ld.gold)
                if(GOLD_EXECUTABLE)
                    message("SIL Kit: LLD not found, using Gold linker: ${GOLD_EXECUTABLE}")
                    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=gold --version 
                        RESULT_VARIABLE GOLD_TEST_RESULT OUTPUT_QUIET ERROR_QUIET)
                    if(GOLD_TEST_RESULT EQUAL 0)
                        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
                        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
                    else()
                        message(WARNING "SIL Kit: Gold linker found but not compatible with this Clang version")
                    endif()
                else()
                    message(WARNING "SIL Kit: Neither LLD nor Gold linker found - using default linker (may cause LTO issues)")
                endif()
            endif()
        endif()
        message(STATUS "SIL Kit: Enabling Link Time Optimization globally.")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "SIL Kit: Disabling Link Time Optimization.")
    endif()

endmacro()
