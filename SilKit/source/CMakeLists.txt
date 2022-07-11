# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_subdirectory(config)
add_subdirectory(tracing)
add_subdirectory(core)
add_subdirectory(services)
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
    message(STATUS "SILKIT re-using deployed version_macros.hpp")
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
        ${INSTALL_SOURCE_DIR}/SilKit/source)
else()
    message(WARNING "Cannot determine hash of current git head!")
    set(GIT_HEAD_HASH "UNKNOWN")
    configure_file(
        version_macros.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        @ONLY)
endif()

set(silkitLibType SHARED)
if(SILKIT_BUILD_STATIC)
    set(silkitLibType STATIC)
endif()

add_library(SilKit ${silkitLibType}
    SilKit.hpp
    SilKit.cpp

    capi/TypeConversion.hpp
    capi/CapiParticipant.cpp
    capi/CapiCan.cpp
    capi/CapiEthernet.cpp
    capi/CapiFlexray.cpp
    capi/CapiLogger.cpp
    capi/CapiLin.cpp
    capi/CapiOrchestration.cpp
    capi/CapiDataPubSub.cpp
    capi/CapiRpc.cpp
    capi/CapiUtils.cpp
    capi/CapiImpl.hpp

    SilKitForceIncludes.cpp
    CreateSilKitRegistry.cpp
)


if(SILKIT_BUILD_STATIC)
    target_compile_definitions(SilKit PUBLIC -D SILKIT_BUILD_STATIC=1)
endif()

if (MSVC)
    target_sources(SilKit PRIVATE SilKit.rc)
    #disable 'truncated names' warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        target_compile_options(SilKit PUBLIC "/wd4503")
    endif()

    #make sure we have the Release PDB in a well known place
    # needed for packaging the PDBs from external
    set_target_properties(SilKit
        PROPERTIES
            PDB_NAME SilKit
            PDB_OUTPUT_DIRECTORY_RELEASE ${PKG_PDB_DIR}/Release
            PDB_OUTPUT_DIRECTORY_DEBUG ${PKG_PDB_DIR}/Debug
            LINK_FLAGS "/DEBUG" #make sure the resulting .dll has a .pdb file
    )
elseif(UNIX)
    #for reproducible builds
    target_link_options(SilKit
        PRIVATE -Wl,--build-id=none)

endif()

 set_target_properties(SilKit PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(SilKit
    PUBLIC
    SilKitInterface
    PRIVATE
    SilKitInternal
    SilKitConfiguration
    SilKitMwParticipant
    SilKitExtensions
    SilKitUtil
)

add_library(SilKitVersion STATIC
    SilKitVersion.cpp
    SilKitVersion.hpp
    ${VERSION_MACROS_HPP}
)
target_link_libraries(SilKitVersion
    PUBLIC  SilKitInterface
)

#add an alias for the exported name which includes a namespace
add_library(SilKit::SilKit ALIAS SilKit)

###############################################################################
# Add to install targets of the SilKit
###############################################################################
# we adhere to standard find_package layout
# refer to https://cmake.org/cmake/help/v3.15/manual/cmake-packages.7.html#package-configuration-file
if(NOT SILKIT_BUILD_STATIC)
install(
    TARGETS SilKit
    EXPORT SilKitTargets
    DESTINATION ${INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${INSTALL_LIB_DIR} # for Linux shared object libraries (.so)
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR} # for static libraries (.lib/.a)
    RUNTIME DESTINATION ${INSTALL_BIN_DIR} # for executables and dynamic libraries (.dll)
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR} # Destination for header files
)
# portable installation  definitions
install(EXPORT SilKitTargets
    DESTINATION ${INSTALL_LIB_DIR}/cmake/SilKit
    NAMESPACE "SilKit::"
    FILE SilKitTargets.cmake
)
endif()
# Also install CmakeTargets for MinSizeRel and RelWithDebInfo
#
#   These files map the targets to the Release build. Without them, applications
#   that are built as MinSizeRel or RelWithDebInfo will link against the Debug
#   build of the SILKIT and will crash.
install(
    FILES
        cmake/SilKitTargets-minsizerel.cmake
        cmake/SilKitTargets-relwithdebinfo.cmake
    DESTINATION ${INSTALL_LIB_DIR}/cmake/SilKit
    CONFIGURATIONS Release
)

#make our local build dir usable for other projects
export(EXPORT SilKitTargets
    NAMESPACE "SilKit::"
    FILE SilKitTargets.cmake
    )
