add_subdirectory(cfg)
add_subdirectory(mw)
add_subdirectory(sim)
add_subdirectory(util)

set(GIT_DIR "${PROJECT_SOURCE_DIR}/../.git")
set(GIT_HEAD_FILE "${GIT_DIR}/HEAD")
if(EXISTS "${GIT_HEAD_FILE}")
    configure_file(
        "MakeVersionMacros.cmake.in"
        ${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake
        @ONLY)
    include(${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake)
else()
    message(WARNING "Cannot determine hash of current git head!")
    set(GIT_HEAD_HASH "UNKNOWN")
    configure_file(
        version_macros.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        @ONLY)
endif()

add_library(IntegrationBus SHARED
    IntegrationBus.hpp
    IntegrationBus.cpp
    IbForceIncludes.cpp
)

if (MSVC)
    target_sources(IntegrationBus PRIVATE IntegrationBus.rc)
endif (MSVC)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(IntegrationBus
    PUBLIC  IbInterface
    PRIVATE IbConfig
    PRIVATE IbMwComAdapter
)

add_library(IbVersion STATIC
    version.cpp
    version.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
)
target_link_libraries(IbVersion
    PUBLIC  IbInterface
)

#add an alias for the exported name which includes a namespace
add_library(IntegrationBus::IntegrationBus ALIAS IntegrationBus)

###############################################################################
# Add to install targets of the IntegrationBus
###############################################################################
# we adhere to standard find_package layout
# refer to https://cmake.org/cmake/help/v3.15/manual/cmake-packages.7.html#package-configuration-file

install(
    TARGETS IntegrationBus
    EXPORT IntegrationBusTargets
    DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR} # for Linux shared object libraries (.so)
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR} # for static libraries (.lib/.a)
    RUNTIME DESTINATION ${INSTALL_BIN_DIR} # for executables and dynamic libraries (.dll)
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR} # Destination for header files
)
# portable installation  definitions
install(EXPORT IntegrationBusTargets
    DESTINATION ${INSTALL_LIB_DIR}/cmake/IntegrationBus
    NAMESPACE "IntegrationBus::"
    FILE IntegrationBusTargets.cmake
)
#make our local build dir usable for other projects
export(EXPORT IntegrationBusTargets
    NAMESPACE "IntegrationBus::"
    FILE IntegrationBusTargets.cmake
    )
#register in CMake user registry
# the following sets registry keys on windows, not good for developing cmake
#export(PACKAGE IntegrationBus)
