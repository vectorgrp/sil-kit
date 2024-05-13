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
option(SILKIT_ENABLE_COVERAGE "Enable coverage for builds (requires gcc, clang)" OFF)
option(SILKIT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(SILKIT_PACKAGE_SYMBOLS "Add a post-build step to create PDB/Symbol archives" ON)
option(SILKIT_BUILD_DASHBOARD "Build the SIL Kit Dashboard client." ON)
option(SILKIT_ENABLE_TRACING_INSTRUMENTATION "Enable tracing instrumentation (_SILKIT_TRACE_CLASS_NAMES)." OFF)
option(SILKIT_LINK_LLD "Use the lld linker for SIL KIT" OFF)
option(SILKIT_USE_SYSTEM_LIBRARIES "Use the libraries installed on the system for third party dependencies" OFF)
option(SILKIT_BUILD_REPRODUCIBLE "Creates a reproducible build by ommiting timestamps/unique build ids" ON)
option(SILKIT_BUILD_LINUX_PACKAGE "Creates SILKIT builds suitable for package managers in Linux Distributions (.deb)" OFF)

if(SILKIT_BUILD_LINUX_PACKAGE)

    message(NOTICE "-- Force Setting variables due to SILKIT_BUILD_LINUX_PACKAGE")

    message(NOTICE "  * SILKIT_BUILD_REPRODUCIBLE   OFF")
    set(SILKIT_BUILD_REPRODUCIBLE OFF CACHE BOOL "Force SILKIT_BUILD_REPRODUCIBLE OFF due to SILKIT_BUILD_LINUX_PACKAGE" FORCE)

    message(NOTICE "  * SILKIT_USE_SYSTEM_LIBRARIES ON")
    set(SILKIT_USE_SYSTEM_LIBRARIES ON CACHE BOOL "Force SILKIT_USE_SYSTEM_LIBRARIES ON due to SILKIT_BUILD_LINUX_PACKAGE" FORCE)

    message(NOTICE "  * SILKIT_BUILD_DASHBOARD      OFF")
    set(SILKIT_BUILD_DASHBOARD OFF CACHE BOOL "Force SILKIT_BUILD_DASHBOARD OFF due to SILKIT_BUILD_REPRODUCIBLE" FORCE)
    add_subdirectory(docs/man)

endif()


set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_compile_definitions(
    ASIO_DISABLE_VISIBILITY=1
)

if(MINGW AND (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang))
    add_compile_options("-Wa,-mbig-obj")
endif()

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

if(SILKIT_ENABLE_TRACING_INSTRUMENTATION)
    add_compile_definitions(SILKIT_ENABLE_TRACING_INSTRUMENTATION=1)
endif()

# Configure build settings like warning and sanitizers
include(SilKitBuildSettings)
silkit_enable_asan(${SILKIT_ENABLE_ASAN})
silkit_enable_ubsan(${SILKIT_ENABLE_UBSAN})
silkit_enable_threadsan(${SILKIT_ENABLE_THREADSAN})
silkit_enable_coverage(${SILKIT_ENABLE_COVERAGE})
silkit_check_reproducible()
silkit_clean_default_compileflags()
silkit_add_libs_for_atomic64()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set compile variables (packaging etc.)
include(get_compiler_arch) 
get_compiler_arch(comp arch plat)
set(SILKIT_HOST_PLATFORM "${plat}")
set(SILKIT_HOST_ARCHITECTURE "${arch}")
set(SILKIT_HOST_COMPILER "${comp}")
message(STATUS "SIL Kit - CMAKE_VERSION ${CMAKE_VERSION}")
message(STATUS "SIL Kit - SILKIT_HOST_PLATFORM ${SILKIT_HOST_PLATFORM}")
message(STATUS "SIL Kit - SILKIT_HOST_ARCHITECTURE ${SILKIT_HOST_ARCHITECTURE}")
message(STATUS "SIL Kit - SILKIT_HOST_COMPILER ${SILKIT_HOST_COMPILER}")

include("SilKit_Platform_${SILKIT_HOST_PLATFORM}" OPTIONAL)

## Internally used in CI builds to archive VS debugging .PDB files:

### in multiconfig builds (vstudio) we cannot rely on CMAKE_BUILD_TYPE to get the build type
### so we expect ci/build_all.sh to set this variable
if(NOT CMAKE_BUILD_TYPE)
    message(WARNING "CMAKE_BUILD_TYPE is not set! cpack files will have no config name.")
    set(my_build_type) #ignore in output file
else()
    set(my_build_type "-${CMAKE_BUILD_TYPE}")
endif()
set(SILKIT_SYMBOLS_DIR_BASE "${CMAKE_BINARY_DIR}")
set(SILKIT_SYMBOLS_DIR_NAME "SilKit-${PROJECT_VERSION}-${SILKIT_HOST_PLATFORM}-${SILKIT_HOST_ARCHITECTURE}-${SILKIT_HOST_COMPILER}${my_build_type}-SYMBOLS")
set(SILKIT_SYMBOLS_DIR "${SILKIT_SYMBOLS_DIR_BASE}/${SILKIT_SYMBOLS_DIR_NAME}")

###############################################################################
# Include the SIL Kit projects to be built
###############################################################################
# Dependencies
add_subdirectory(ThirdParty)
silkit_add_third_party_packages()

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
if(SILKIT_BUILD_DOCS)
    add_subdirectory(docs)
endif()

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
        COMPONENT source
        FILES_MATCHING
            PATTERN *.cpp
            PATTERN *.cxx
            PATTERN *.c
            PATTERN *.rc
            PATTERN *.map
            PATTERN *.h
            PATTERN *.hpp
            PATTERN *.hpp.in
            PATTERN *.ipp
            PATTERN *.json
            PATTERN *.yaml
            PATTERN *.cmake
            PATTERN *.cmake.in
            PATTERN CMakeLists.txt
            PATTERN Readme-*.txt
            PATTERN *.txt.in
            PATTERN *.md
            PATTERN *.xml
            PATTERN *.manifest
        REGEX "/ci/.*" EXCLUDE
        REGEX "/ci$" EXCLUDE
        REGEX "\.git$" EXCLUDE
        REGEX "\.github$" EXCLUDE
    )

    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/Demos/
        DESTINATION ${INSTALL_DEMO_DIR}
        COMPONENT source
    )

    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/docs/
        DESTINATION ${INSTALL_SOURCE_DIR}/docs
        COMPONENT source
    )

    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/
        DESTINATION ${INSTALL_SOURCE_DIR}/ThirdParty
        COMPONENT source
        REGEX "\.git$" EXCLUDE
        REGEX "\.github$" EXCLUDE
    )

    install(
        FILES
            CMakeLists.txt
            CMakePresets.json
        DESTINATION ${INSTALL_SOURCE_DIR}
        COMPONENT source
    )

    # create a top-level README.txt with some pointers to the contained packages
    configure_file(
        cmake/README.txt.in
        TOP_LEVEL_README.txt
        @ONLY
        )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TOP_LEVEL_README.txt
        COMPONENT source
        DESTINATION .
        RENAME README.txt
        )

    # make sure we have our license and copyright infos packaged
    install(FILES
        LICENSE
        COMPONENT source
        DESTINATION .
        )
endif(SILKIT_INSTALL_SOURCE)


###############################################################################
# Packaging
###############################################################################
#set(CPACK_GENERATOR "ZIP;DEB")
set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
#set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_DESCRIPTION "binary release of SIL Kit library and tools")
set(CPACK_PACKAGE_NAME "SilKit")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME 
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${plat}-${arch}-${comp}${my_build_type}")
set(CPACK_PACKAGE_CONTACT "support@vector.com")

set(CPACK_ARCHIVE_BIN_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}") 
set(CPACK_ARCHIVE_SOURCE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
set(CPACK_ARCHIVE_DOCS_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Docs")
set(CPACK_COMPONENT_UNSPECIFIED_DISABLED ON)
set(CPACK_COMPONENTS_ALL "bin")
list(APPEND CPACK_COMPONENTS_ALL "dev")
list(APPEND CPACK_COMPONENTS_ALL "utils")


if(SILKIT_BUILD_DOCS)
    list(APPEND CPACK_COMPONENTS_ALL "docs")
endif()
if(SILKIT_INSTALL_SOURCE)
    list(APPEND CPACK_COMPONENTS_ALL "source")
endif()

include(CPack)

cpack_add_component(docs DISPLAY_NAME "SIL Kit Documentation")
cpack_add_component(source DISPLAY_NAME "SIL Kit Sources")
cpack_add_component(bin DISPLAY_NAME "SIL Kit Binaries")
cpack_add_component(dev DISPLAY_NAME "SIL Kit Headers")
cpack_add_component(utils DISPLAY_NAME "SIL Kit Utility Tools")
