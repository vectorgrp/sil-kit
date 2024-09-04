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

cmake_minimum_required(VERSION 3.13)

################################################################################
# Project definition
################################################################################

project("SilKit-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(SilKitVersion)
configure_silkit_version(${PROJECT_NAME})

set(SILKIT_PROJECT_DIR ${PROJECT_SOURCE_DIR})

silkit_clean_default_compileflags()

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

add_silkit_test_executable(SilKitUnitTests)
add_silkit_test_executable(SilKitIntegrationTests)
add_silkit_test_executable(SilKitInternalIntegrationTests)
add_silkit_test_executable(SilKitFunctionalTests)
add_silkit_test_executable(SilKitInternalFunctionalTests)

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
    COMPONENT dev
    )
#create a portable packageConfig.cmake
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/SilKitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfig.cmake
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    INSTALL_DESTINATION ${_DEPLOY_CMAKE_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SilKitConfig.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    COMPONENT dev
    )

