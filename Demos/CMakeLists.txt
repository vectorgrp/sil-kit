# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    include(SilKitBuildTools)
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

set(SILKIT_DEMO_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#######################################################################################################################
# Demo creation macros
#######################################################################################################################

macro(make_silkit_demo executableName demoSourceFile addToPackage)

    add_executable(${executableName}
        ${demoSourceFile}
        $<$<CXX_COMPILER_ID:MSVC>: "${CMAKE_CURRENT_SOURCE_DIR}/../../demo.manifest" >
    )
    remove_definitions(-DEXPORT_SilKitAPI)
    
    # Group this demo project into a folder
    set_target_properties(${executableName} PROPERTIES
        FOLDER "Demos"
        RUNTIME_OUTPUT_DIRECTORY "${_DEMO_OUTPUT_DIR}"
        INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
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
    if(MSVC)
        target_compile_options(${executableName} PRIVATE /wd4996)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        target_compile_definitions(${executableName} PRIVATE -D SILKIT_BUILD_IS_DEBUG)
    endif()

    add_dependencies(Demos ${executableName})

    if(${addToPackage})
        install(TARGETS ${executableName}
            RUNTIME DESTINATION ${INSTALL_BIN_DIR}
            COMPONENT demos
        )
    endif()
    if(NOT _is_standalone)
        silkit_strip_distrib(${executableName})
    endif()
endmacro()

macro(make_silkit_communication_demo executableName demoSourceFile)

    make_silkit_demo(${executableName} ${demoSourceFile} ON)
    target_include_directories(${executableName} PRIVATE ${SILKIT_DEMO_DIR}/communication/include)

endmacro()

#######################################################################################################################
# Add the actual demo projects
#######################################################################################################################

add_subdirectory(communication/Can)
add_subdirectory(communication/Ethernet)
add_subdirectory(communication/Flexray)
add_subdirectory(communication/Lin)
add_subdirectory(communication/PubSub)
add_subdirectory(communication/Rpc)

add_subdirectory(tools/Benchmark)

add_subdirectory(api/SimpleCan)
add_subdirectory(api/NetworkSimulator)
add_subdirectory(api/Orchestration)

#######################################################################################################################
# VisualStudio specific setup
#######################################################################################################################

if(MSVC)
    # Create and deploy VisualStudio debug information files for all targets
    add_compile_options("/Zi")
endif(MSVC)
