cmake_minimum_required(VERSION 3.11)

################################################################################
# Project definition
################################################################################

project("IntegrationBus-Library" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusVersion)
configure_ib_version(${PROJECT_NAME})

set(IB_PROJECT_DIR ${PROJECT_SOURCE_DIR})

option(IB_INSTALL_PDB_FILES "Install Visual Studio debug symbol files along with the IB dll" OFF)

if(MSVC)
    add_compile_options("/MP;/W4;/w44062")
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()
endif (MSVC)

add_definitions(-DEXPORT_IntegrationBusAPI)
## Allow debug libraries to be installed side-by-side with their release variant
set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Make sure debug binaries differ from release binaries by a suffix 'd'" FORCE)

################################################################################
# Install definitions
################################################################################

include(IntegrationBusInstall)

# Test related tools
include(IntegrationBusTest)
include(CTest)

################################################################################
# Include of our repositories
################################################################################

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(IntegrationTests)

set(_DEPLOY_CMAKE_DIR  ${INSTALL_LIB_DIR}/cmake/IntegrationBus/)

# export our library version for usage in find_package() by a user of the library
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfigVersion.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )
#create a portable packageConfig.cmake
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/IntegrationBusConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    INSTALL_DESTINATION ${_DEPLOY_CMAKE_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/IntegrationBusConfig.cmake
    DESTINATION ${_DEPLOY_CMAKE_DIR}
    )

