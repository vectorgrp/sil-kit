# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.5)

###############################################################################
# Project definition
###############################################################################

project("IntegrationBus")

option(IB_BUILD_DEMOS "Build the IntegrationBus Demos" ON)
option(IB_BUILD_STATIC "Compile the Integration Bus as a static library" OFF)
option(IB_BUILD_TESTS "Enable unit and integration tests for the Integration Bus" ON)
option(IB_BUILD_DOCS "Build documentation for the Integration Bus (requires Doxygen and Sphinx)" OFF)
option(IB_BUILD_DOCS_NEW "Build documentation for VIB 4.0" OFF)
option(IB_INSTALL_SOURCE "Install and package the source tree" OFF)
if(IB_INSTALL_SOURCE)
    set(IB_BUILD_DOCS ON CACHE BOOL "Force IB_BUILD_DOCS ON due to IB_INSTALL_SOURCE" FORCE)
endif()
option(IB_ENABLE_ASAN "Enable -f sanitize=address for builds (requires gcc, clang,  VS2019)" OFF)
option(IB_ENABLE_UBSAN "Enable -f sanitize=undefined for builds (requires gcc, clang)" OFF)
option(IB_ENABLE_THREADSAN "Enable -f sanitize=thread for builds (requires gcc, clang)" OFF)
option(IB_MW_ENABLE_VASIO "Build VAsio Middleware" ON)
option(IB_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_CURRENT_LIST_DIR}/IntegrationBus/cmake)
include(IntegrationBusInstall)
include(IntegrationBusVersion)
configure_ib_version(${PROJECT_NAME})

# Enable testing for this project
enable_testing()

# Global build settings
if(MSVC)
    #make sure Release is built with debug PDBs
    add_compile_options(/Zi )
endif()

# Configure build settings like warning and sanitizers
include(IntegrationBusBuildSettings)
ib_enable_asan(${IB_ENABLE_ASAN})
ib_enable_ubsan(${IB_ENABLE_UBSAN})
ib_enable_threadsan(${IB_ENABLE_THREADSAN})
ib_check_reproducible()
ib_clean_default_compileflags()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Globally add -fPIC compiler option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Include the IntegrationBus projects to be built
###############################################################################
# Dependencies
add_subdirectory(ThirdParty)

# Globally set the warning compile options AFTER including ThirdParty
ib_enable_warnings(${IB_WARNINGS_AS_ERRORS})

# Have both IntegrationBus library and demo project in a single solution
add_subdirectory(IntegrationBus)
add_subdirectory(Utilities)
if(IB_BUILD_DEMOS)
    add_subdirectory(Demos)
endif()
# Include the Python-implemented launcher tool for testing and installation
add_subdirectory(Launcher)
# Include automated documentation with doxygen and sphinx
add_subdirectory(docs)
add_subdirectory(docs_new)

################################################################################
# Distribution of the source code
################################################################################
# Install sources
# Copy all files from the source directory to the proper destination
# Filter by file types in case someone spoiled this source folder by calling "cmake ."
if(IB_INSTALL_SOURCE)
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake
            ${CMAKE_CURRENT_SOURCE_DIR}/Demos
            ${CMAKE_CURRENT_SOURCE_DIR}/IntegrationBus
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
            ${CMAKE_CURRENT_SOURCE_DIR}/Launcher
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
        COMPONENT IntegrationBus-Developer
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
        IntegrationBus/LICENSE
        DESTINATION .)
endif(IB_INSTALL_SOURCE)


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
set(CPACK_PACKAGE_DESCRIPTION "binary release of IntegrationBus library and tools")
set(CPACK_PACKAGE_NAME "IntegrationBus")
set(CPACK_PACKAGE_VENDOR "Vector Informatik")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME 
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${comp}-${plat}${my_build_type}")
include(CPack)
