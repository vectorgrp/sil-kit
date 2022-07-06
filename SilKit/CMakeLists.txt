# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.11)

################################################################################
# Project definition
################################################################################

project("SilKit-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(SilKitVersion)
configure_silkit_version(${PROJECT_NAME})

set(SILKIT_PROJECT_DIR ${PROJECT_SOURCE_DIR})

option(SILKIT_INSTALL_PDB_FILES "Install Visual Studio debug symbol files along with the SilKit dll" OFF)

if(MSVC)
    add_compile_options("/MP;/w44062")
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()
endif (MSVC)

if(SILKIT_BUILD_STATIC)
    add_definitions(-D SILKIT_BUILD_STATIC)
else()
    add_definitions(-D EXPORT_SilKitAPI)
endif()
## Allow debug libraries to be installed side-by-side with their release variant
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Make sure debug binaries differ from release binaries by a suffix 'd'" FORCE)

################################################################################
# Install definitions
################################################################################

include(SilKitInstall)

# Test related tools
include(SilKitTest)
include(CTest)

################################################################################
# Include of our repositories
################################################################################

add_subdirectory(include)
add_subdirectory(source)
if(SILKIT_BUILD_TESTS)
    add_subdirectory(IntegrationTests)
endif()

set(_DEPLOY_CMAKE_DIR  ${INSTALL_LIB_DIR}/cmake/SilKit/)

# export our library version for usage in find_package() by a user of the library
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfigVersion.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )
#create a portable packageConfig.cmake
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/SilKitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfig.cmake
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    INSTALL_DESTINATION ${_DEPLOY_CMAKE_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfig.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )

