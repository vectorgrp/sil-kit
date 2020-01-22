cmake_minimum_required(VERSION 3.5)

###############################################################################
# Project definition
###############################################################################

project("IntegrationBus")

option(IB_BUILD_TESTS "Enable unit and integration tests for the Integration Bus" ON)
option(IB_BUILD_DOCS "Build documentation for the Integration Bus (requires Doxygen and Sphinx)" OFF)
option(IB_INSTALL_SOURCE "Install and package the source tree" OFF)
if(IB_INSTALL_SOURCE)
    set(IB_BUILD_DOCS ON CACHE BOOL "Force IB_BUILD_DOCS ON due to IB_INSTALL_SOURCE" FORCE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake ${CMAKE_CURRENT_LIST_DIR}/IntegrationBus/cmake)
include(IntegrationBusInstall)
include(IntegrationBusVersion)
configure_ib_version(${PROJECT_NAME})

# Enable testing for this project
enable_testing()

###############################################################################
# Include the IntegrationBus projects to be built
###############################################################################
# Have both IntegrationBus library and demo project in a single solution
add_subdirectory(${PROJECT_SOURCE_DIR}/IntegrationBus)
add_subdirectory(${PROJECT_SOURCE_DIR}/Utilities)
add_subdirectory(${PROJECT_SOURCE_DIR}/Demos)
# Include the Python-implemented launcher tool for testing and installation
add_subdirectory(${PROJECT_SOURCE_DIR}/Launcher)
# Include automated documentation with doxygen and sphinx
add_subdirectory (${PROJECT_SOURCE_DIR}/docs)


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
            PATTERN *.rc
            PATTERN *.h
            PATTERN *.hpp
            PATTERN *.hpp.in
            PATTERN *.json
            PATTERN *.cmake
            PATTERN *.cmake.in
            PATTERN CMakeLists.txt
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
        REGEX "Fast-RTPS" EXCLUDE
        REGEX "\.git$" EXCLUDE
    )
    #bundle ASIO
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Fast-RTPS/thirdparty/asio
        DESTINATION ${INSTALL_SOURCE_DIR}/ThirdParty/Fast-RTPS/thirdparty
        REGEX "\.git$" EXCLUDE
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
