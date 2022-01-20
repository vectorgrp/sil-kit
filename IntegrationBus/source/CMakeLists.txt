# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_subdirectory(cfg)
add_subdirectory(tracing)
add_subdirectory(mw)
add_subdirectory(sim)
add_subdirectory(util)
add_subdirectory(extensions)
add_subdirectory(capi)

# Encode the current GIT and version infos into version_macros.hpp
# If this file is present in the current source directory we assume
# that this source-tree is a non-git/packaged source tree and re-use
# the file unmodified.
set(GIT_DIR "${PROJECT_SOURCE_DIR}/../.git")
set(GIT_HEAD_FILE "${GIT_DIR}/HEAD")
set(VERSION_MACROS_HPP ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp)
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/version_macros.hpp)
    message(STATUS "VIB re-using deployed version_macros.hpp")
    set(VERSION_MACROS_HPP ${CMAKE_CURRENT_LIST_DIR}/version_macros.hpp)
elseif(EXISTS "${GIT_HEAD_FILE}")
    configure_file(
        "MakeVersionMacros.cmake.in"
        ${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake
        @ONLY)
    include(${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        DESTINATION
        ${INSTALL_SOURCE_DIR}/IntegrationBus/source)
else()
    message(WARNING "Cannot determine hash of current git head!")
    set(GIT_HEAD_HASH "UNKNOWN")
    configure_file(
        version_macros.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        @ONLY)
endif()

set(ibLibType SHARED)
if(IB_BUILD_STATIC)
    set(ibLibType STATIC)
endif()

add_library(IntegrationBus ${ibLibType}
    IntegrationBus.hpp
    IntegrationBus.cpp
    "capi/CapiSimulationParticipant.cpp"
    "capi/CapiCan.cpp"
    "capi/CapiEthernet.cpp"
    "capi/CapiFlexRay.cpp"
    "capi/CapiLogger.cpp"
    "capi/CapiLin.cpp"
    "capi/CapiDataPubSub.cpp"
    "capi/CapiUtils.cpp"
    "capi/CapiImpl.h"
    IbForceIncludes.cpp
)

if(IB_BUILD_STATIC)
    target_compile_definitions(IntegrationBus PUBLIC -D IB_BUILD_STATIC=1)
    target_compile_definitions(IntegrationBus PUBLIC -D CIntegrationBusAPI_STATIC=1)
else()
    target_compile_definitions(IntegrationBus PUBLIC -D CIntegrationBusAPI_EXPORT=1)
endif()

# Published extension implementations follow here for the extension loading
# side of the code. The actual shared library code might reside 
# anywhere.
# Note to future self:
#    It would be desirable to have the extension loading code inside of the
#    IbExtension target. However, this causes problems when exporting the
#    IntegrationBus target, as it would transitively export the private
#    IbExtension library and other internal dependencies.

target_sources(IntegrationBus
    PRIVATE
    #for IDE:
    ${CMAKE_CURRENT_LIST_DIR}/../include/ib/extensions/IIbRegistry.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../include/ib/extensions/CreateExtension.hpp
    # implementations accessible via the public API
    extensions/IbExtensionImpl/CreateIbRegistry.cpp
)

if (MSVC)
    target_sources(IntegrationBus PRIVATE IntegrationBus.rc)
    #disable 'truncated names' warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        target_compile_options(IntegrationBus PUBLIC "/wd4503")
    endif()

    #make sure we have the Release PDB in a well known place
    # needed for packaging the PDBs from external
    set_target_properties(IntegrationBus
        PROPERTIES
            PDB_NAME IntegrationBus
            PDB_OUTPUT_DIRECTORY_RELEASE ${PKG_PDB_DIR}/Release
            PDB_OUTPUT_DIRECTORY_DEBUG ${PKG_PDB_DIR}/Debug
            LINK_FLAGS "/DEBUG" #make sure the resulting .dll has a .pdb file
    )
elseif(UNIX)
    #for reproducible builds
    target_link_options(IntegrationBus 
        PRIVATE -Wl,--build-id=none)
    # strip symbols
    #if(CMAKE_BUILD_TYPE STREQUAL Release)
    #    add_custom_command(TARGET IntegrationBus
    #        POST_BUILD
    #        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:IntegrationBus>)
    #endif()

endif()

 set_target_properties(IntegrationBus PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(IntegrationBus
    PUBLIC
    IbInterface
    PRIVATE
    IbInternal
    IbConfig
    IbMwComAdapter
    IbExtensions
)

add_library(IbVersion STATIC
    version.cpp
    version.hpp
    ${VERSION_MACROS_HPP}
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
if(NOT IB_BUILD_STATIC)
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
endif()
# Also install CmakeTargets for MinSizeRel and RelWithDebInfo
#
#   These files map the targets to the Release build. Without them, applications
#   that are built as MinSizeRel or RelWithDebInfo will link against the Debug
#   build of the VIB and will crash.
install(
    FILES
        cmake/IntegrationBusTargets-minsizerel.cmake
        cmake/IntegrationBusTargets-relwithdebinfo.cmake
    DESTINATION ${INSTALL_LIB_DIR}/cmake/IntegrationBus
    CONFIGURATIONS Release
)

#make our local build dir usable for other projects
export(EXPORT IntegrationBusTargets
    NAMESPACE "IntegrationBus::"
    FILE IntegrationBusTargets.cmake
    )
#register in CMake user registry
# the following sets registry keys on windows, not good for developing cmake
#export(PACKAGE IntegrationBus)
