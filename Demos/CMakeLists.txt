# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.5)

#######################################################################################################################
# Project definition
#######################################################################################################################
project("SilKit-Demos" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

set(_is_standalone FALSE)
if(NOT TARGET SilKit)
    #assuming we are standalone but still within the original distribution directory layout
    #so the source release should be located in ../${INSTALL_SOURCE_DIR}
    message(STATUS "<-> no SilKit-Library target found. working in distribution mode")
    message(STATUS "<-> using deployed SilKit")
    #NB: as of SILKIT 3.0.2 FMT is optional
    include(
        ${CMAKE_CURRENT_LIST_DIR}/../SilKit-Source/ThirdParty/fmt.cmake
        OPTIONAL)
    find_package(SilKit
        REQUIRED
        CONFIG
        NO_CMAKE_PACKAGE_REGISTRY
        NO_DEFAULT_PATH
        PATHS "${CMAKE_CURRENT_LIST_DIR}/../SilKit"
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
        message(STATUS "Searching for SilKit library at prefix path: " ${path})
    endforeach(path)
    message(STATUS "SilKit-Demos: Note: Set '-DCMAKE_PREFIX_PATH=<path>' so that ./cmake/SilKitTargets.cmake resolves.")
    find_package(SilKit ${PROJECT_VERSION} REQUIRED)
else()
    message(STATUS "Linking to SilKit library that is built in scope of root project")
endif()

#######################################################################################################################
# Helper Functions
#######################################################################################################################

set(make_silkit_demo_caller_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

set(demo_list)

add_custom_target(Demos COMMENT "Build all available Demos")
set_property(TARGET Demos PROPERTY FOLDER "Demos")

# Generate SilKit Demo Project
function(make_silkit_demo executableName demoSourceFile)
    add_executable(${executableName}
        ${demoSourceFile}
    )
    if(_is_standalone)
        set(_DEMO_OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../SilKit/bin")
    else()
        set(_DEMO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>")
    endif()
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
endfunction(make_silkit_demo)

#######################################################################################################################
# Add the actual demo projects
#######################################################################################################################
# C++ Demos
add_subdirectory(Can)
add_subdirectory(Ethernet)
add_subdirectory(Flexray)
add_subdirectory(Lin)
add_subdirectory(DataMessage)
add_subdirectory(Rpc)
add_subdirectory(Benchmark)
add_subdirectory(Lifecycle)

# C-API Demos
add_subdirectory(CCan)
add_subdirectory(CFlexray)
add_subdirectory(CEthernet)
add_subdirectory(CLin)
add_subdirectory(CData)
add_subdirectory(CRpc)

message(STATUS "Demos available: ${demo_list}")
#######################################################################################################################
# VisualStudio specific setup
#######################################################################################################################

if(MSVC)
    # Create and deploy VisualStudio debug information files for all targets
    add_compile_options("/Zi")
endif(MSVC)
