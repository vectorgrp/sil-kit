# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.5)

###############################################################################
# Project definition
###############################################################################

project("SilKit")

option(SILKIT_BUILD_DEMOS "Build the SIL Kit Demos" ON)
option(SILKIT_BUILD_STATIC "Compile the SIL Kit as a static library" OFF)
option(SILKIT_BUILD_TESTS "Enable unit and integration tests for the SIL Kit" ON)
option(SILKIT_BUILD_UTILITIES "Build the SIL Kit utility tools" ON)
option(SILKIT_BUILD_DOCS "Build documentation for the SIL Kit (requires Doxygen and Sphinx)" OFF)
option(SILKIT_INSTALL_SOURCE "Install and package the source tree" OFF)
if(SILKIT_INSTALL_SOURCE)
    set(SILKIT_BUILD_DOCS ON CACHE BOOL "Force SILKIT_BUILD_DOCS ON due to SILKIT_INSTALL_SOURCE" FORCE)
endif()
option(SILKIT_ENABLE_ASAN "Enable -f sanitize=address for builds (requires gcc, clang,  VS2019)" OFF)
option(SILKIT_ENABLE_UBSAN "Enable -f sanitize=undefined for builds (requires gcc, clang)" OFF)
option(SILKIT_ENABLE_THREADSAN "Enable -f sanitize=thread for builds (requires gcc, clang)" OFF)
option(SILKIT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_CURRENT_LIST_DIR}/SilKit/cmake)
include(SilKitInstall)
include(SilKitVersion)
configure_silkit_version(${PROJECT_NAME})

# Enable testing for this project
enable_testing()

# Global build settings
if(MSVC)
    #make sure Release is built with debug PDBs
    add_compile_options(/Zi )
endif()

# Configure build settings like warning and sanitizers
include(SilKitBuildSettings)
silkit_enable_asan(${SILKIT_ENABLE_ASAN})
silkit_enable_ubsan(${SILKIT_ENABLE_UBSAN})
silkit_enable_threadsan(${SILKIT_ENABLE_THREADSAN})
silkit_check_reproducible()
silkit_clean_default_compileflags()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Include the SIL Kit projects to be built
###############################################################################
# Dependencies
add_subdirectory(ThirdParty)

# Globally set the warning compile options AFTER including ThirdParty
silkit_enable_warnings(${SILKIT_WARNINGS_AS_ERRORS})

if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "") #skip 'lib' prefix for extensions and SIL Kit
    set(CMAKE_STATIC_LIBRARY_PREFIX "") #skip 'lib' prefix for import libs
    # Debug builds grow too big for the assembler, add flags to reduce size
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O2 -Wa,-mbig-obj")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2  -Wa,-mbig-obj")
    endif()
endif()

# Have both SIL Kit library and demo project in a single solution
add_subdirectory(SilKit)
if(SILKIT_BUILD_UTILITIES)
    add_subdirectory(Utilities)
endif()
if(SILKIT_BUILD_DEMOS)
    add_subdirectory(Demos)
endif()
# Include automated documentation with doxygen and sphinx
add_subdirectory(docs)

################################################################################
# Distribution of the source code
################################################################################
# Install sources
# Copy all files from the source directory to the proper destination
# Filter by file types in case someone spoiled this source folder by calling "cmake ."
if(SILKIT_INSTALL_SOURCE)
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake
            ${CMAKE_CURRENT_SOURCE_DIR}/Demos
            ${CMAKE_CURRENT_SOURCE_DIR}/SilKit
            ${CMAKE_CURRENT_SOURCE_DIR}/Utilities
        DESTINATION ${INSTALL_SOURCE_DIR}
        FILES_MATCHING
            PATTERN *.cpp
            PATTERN *.cxx
            PATTERN *.c
            PATTERN *.rc
            PATTERN *.h
            PATTERN *.hpp
            PATTERN *.hpp.in
            PATTERN *.json
            PATTERN *.yaml
            PATTERN *.cmake
            PATTERN *.cmake.in
            PATTERN CMakeLists.txt
            PATTERN Readme-*.txt
            PATTERN *.txt.in
            PATTERN *.md
            PATTERN *.xml
        REGEX "/ci/.*" EXCLUDE
    )
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/Demos/
        DESTINATION ${INSTALL_DEMO_DIR}
    )
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/docs
        DESTINATION ${INSTALL_SOURCE_DIR}
    )
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/
        DESTINATION ${INSTALL_SOURCE_DIR}/ThirdParty
        REGEX "breathe" EXCLUDE
        REGEX "\.git$" EXCLUDE
        REGEX "\.github$" EXCLUDE
    )
    install(
        FILES
            CMakeLists.txt
            CMakeSettings.json
        DESTINATION ${INSTALL_SOURCE_DIR}
        COMPONENT SilKit-Developer
    )

    # create a top-level README.txt with some pointers to the contained packages
    configure_file(
        cmake/README.txt.in
        TOP_LEVEL_README.txt
        @ONLY
        )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TOP_LEVEL_README.txt
        DESTINATION .
        RENAME README.txt)

    # make sure we have our license and copyright infos packaged
    install(FILES
        SilKit/LICENSE
        DESTINATION .)
endif(SILKIT_INSTALL_SOURCE)


###############################################################################
# Packaging
###############################################################################
include(get_compiler_arch) 
get_compiler_arch(comp arch plat)

if(UNIX)
    get_ubuntu_version(ubuntuVersion)
    if(NOT ${ubuntuVersion} MATCHES "18.04")
        # ensure that the build artifact does not override the default of 18.04
        # on our CI
        set(plat "${plat}_${ubuntuVersion}")
    endif()
endif()

### in multiconfig builds (vstudio) we cannot rely on CMAKE_BUILD_TYPE to get the build type
### so we expect ci/build_all.sh to set this variable
if(NOT CMAKE_BUILD_TYPE)
    message(WARNING "CMAKE_BUILD_TYPE is not set! cpack files will have no config name.")
    set(my_build_type) #ignore in output file
else()
    set(my_build_type "-${CMAKE_BUILD_TYPE}")
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_DESCRIPTION "binary release of SIL Kit library and tools")
set(CPACK_PACKAGE_NAME "SilKit")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME 
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${comp}-${plat}${my_build_type}")
include(CPack)
