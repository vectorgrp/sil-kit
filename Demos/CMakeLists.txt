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

cmake_minimum_required(VERSION 3.12)

#######################################################################################################################
# Project definition
#######################################################################################################################
project("SilKit-Demos" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

set(_is_standalone FALSE)
if(NOT TARGET SilKit)
    # Assuming we are standalone but still within the original distribution directory layout
    # so the source release should be located in ../${INSTALL_SOURCE_DIR}
    message(STATUS "<-> no SilKit-Library target found. working in distribution mode")
    message(STATUS "<-> Using a prebuilt SIL Kit package. If find_package fails,"
            " please specify SILKIT_DIR to provide a look up hint.")

    file(GLOB _glob_silkit_dir LIST_DIRECTORIES true
        "${CMAKE_CURRENT_LIST_DIR}/../../SilKit-*.*.*-*-*-*/include/"
    )
    set(silkitDirs)
    foreach(path  IN LISTS _glob_silkit_dir)
        list(APPEND silkitDirs "${path}/..")
    endforeach()

    find_package(SilKit
        REQUIRED
        CONFIG
        NO_CMAKE_PACKAGE_REGISTRY
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_LIST_DIR}/../SilKit"
            "${SILKIT_DIR}"
            ${silkitDirs}
    )
    set(_is_standalone TRUE)
else()
    include(SilKitInstall)
    include(SilKitVersion)
    configure_silkit_version(${PROJECT_NAME})
endif()


#######################################################################################################################
# Dependencies
#######################################################################################################################

# Try to resolve from installed packages only if not defined previously via add_library
if(NOT TARGET SilKit::SilKit)
    foreach(path ${CMAKE_PREFIX_PATH})
        message(STATUS "Searching for SIL Kit library at prefix path: " ${path})
    endforeach(path)
    message(STATUS "SilKit-Demos: Note: Set '-DCMAKE_PREFIX_PATH=<path>' so that ./cmake/SilKitTargets.cmake resolves.")
    find_package(SilKit ${PROJECT_VERSION} REQUIRED)
else()
    message(STATUS "Linking to SIL Kit library that is built in scope of root project")
endif()

#######################################################################################################################
# Helper Functions
#######################################################################################################################

set(make_silkit_demo_caller_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

set(demo_list "")

add_custom_target(Demos COMMENT "Build all available Demos")
set_property(TARGET Demos PROPERTY FOLDER "Demos")

# Generate SIL Kit Demo Project

if(_is_standalone)
    get_target_property(silkitImportLocation SilKit::SilKit IMPORTED_LOCATION_RELEASE)
    get_filename_component(silkitLibDir "${silkitImportLocation}" DIRECTORY)
    set(_DEMO_OUTPUT_DIR "${silkitLibDir}/../bin")
else()
    set(_DEMO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>")
endif()

macro(make_silkit_demo executableName demoSourceFile)
    add_executable(${executableName}
        ${demoSourceFile}
    )
    remove_definitions(-DEXPORT_SilKitAPI)
    
    # Group this demo project into a folder
    set_target_properties(${executableName} PROPERTIES
        FOLDER "Demos"
        RUNTIME_OUTPUT_DIRECTORY "${_DEMO_OUTPUT_DIR}"
        )

    foreach(_build_type ${CMAKE_CONFIGURATION_TYPES})
        string( TOUPPER ${_build_type} _build_type )
        set_target_properties(${executableName} PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${_build_type}" "${_DEMO_OUTPUT_DIR}"
            "LIBRARY_OUTPUT_DIRECTORY_${_build_type}" "${_DEMO_OUTPUT_DIR}"
            "ARCHIVE_OUTPUT_DIRECTORY_${_build_type}" "${_DEMO_OUTPUT_DIR}")
    endforeach()

    find_package(Threads)
    target_link_libraries(${executableName}
        PRIVATE 
            SilKit::SilKit 
            Threads::Threads
    )
    add_dependencies(Demos ${executableName})
    set(demo_list "${demo_list} ${executableName}" PARENT_SCOPE)
endmacro()

#######################################################################################################################
# Add the actual demo projects
#######################################################################################################################
# C++ Demos
add_subdirectory(Can)
add_subdirectory(Ethernet)
add_subdirectory(Flexray)
add_subdirectory(Lin)
add_subdirectory(PubSub)
add_subdirectory(Rpc)
add_subdirectory(Benchmark)

message(STATUS "Demos available: ${demo_list}")
#######################################################################################################################
# VisualStudio specific setup
#######################################################################################################################

if(MSVC)
    # Create and deploy VisualStudio debug information files for all targets
    add_compile_options("/Zi")
endif(MSVC)
