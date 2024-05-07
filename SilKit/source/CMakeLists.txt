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


add_subdirectory(util)
add_subdirectory(wire)
add_subdirectory(config)
add_subdirectory(tracing)
add_subdirectory(core)
add_subdirectory(services)
add_subdirectory(extensions)
add_subdirectory(capi)
add_subdirectory(experimental)
add_subdirectory(dashboard)

find_package(Threads REQUIRED)

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
    if(SILKIT_INSTALL_SOURCE)
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
            DESTINATION
            ${INSTALL_SOURCE_DIR}/SilKit/source
            COMPONENT source
        )
    endif()
else()
    message(WARNING "Cannot determine hash of current git head!")
    set(GIT_HEAD_HASH "UNKNOWN")
    configure_file(
        version_macros.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        @ONLY)
endif()
# now install the configured version_macros.hpp as a header into the `include/silkit/capi` directory.
# NB please keep the version_macros.hpp C11 compatible!
install(FILES
    ${VERSION_MACROS_HPP}
    DESTINATION ${INSTALL_INCLUDE_DIR}/silkit/capi
    RENAME SilKitVersionMacros.h
    COMPONENT dev
)

set(silkitLibType SHARED)
if(SILKIT_BUILD_STATIC)
    set(silkitLibType STATIC)
endif()


add_library(I_SilKit_CreateParticipantImpl INTERFACE)

target_include_directories(I_SilKit_CreateParticipantImpl
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(O_SilKit_CreateParticipantImpl OBJECT
    CreateParticipantImpl.hpp
    CreateParticipantImpl.cpp
)

target_link_libraries(O_SilKit_CreateParticipantImpl
    PUBLIC I_SilKit_CreateParticipantImpl
    PUBLIC SilKitInterface

    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Core_Internal

    PRIVATE O_SilKit_Core_Participant
)


add_library(I_SilKit_CreateSilKitRegistryImpl INTERFACE)

target_include_directories(I_SilKit_CreateSilKitRegistryImpl
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(O_SilKit_CreateSilKitRegistryImpl OBJECT
    CreateSilKitRegistryImpl.hpp
    CreateSilKitRegistryImpl.cpp

    CreateSilKitRegistryWithDashboard.hpp
    CreateSilKitRegistryWithDashboard.cpp
)

target_link_libraries(O_SilKit_CreateSilKitRegistryImpl
    PUBLIC I_SilKit_CreateSilKitRegistryImpl
    PUBLIC SilKitInterface

    PRIVATE O_SilKit_Core_VAsio
)


add_library(I_SilKit_VersionImpl INTERFACE)
target_include_directories(I_SilKit_VersionImpl
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(O_SilKit_VersionImpl OBJECT
    SilKitVersionImpl.cpp
    SilKitVersionImpl.hpp
    ${VERSION_MACROS_HPP}
)
target_link_libraries(O_SilKit_VersionImpl
    PUBLIC I_SilKit_VersionImpl
    PUBLIC SilKitInterface
)


add_library(O_SilKitLegacyAbi OBJECT
    # Exported "Legacy" ABI Functions:
    # - SilKit::Config::ParticipantConfigurationFromFile
    # - SilKit::Config::ParticipantConfigurationFromString
    # - SilKit::Experimental::Participant::CreateSystemController
    # - SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandler
    # - SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler
    # - SilKit::Experimental::Services::Lin::GetSlaveConfiguration
    # - SilKit::Vendor::Vector::CreateSilKitRegistry
    # - SilKit::Version::{Major, Minor, Patch, BuildNumber, String, VersionSuffix, GitHash}
    # - SilKit::CreateParticipant (x2)
    LegacyAbi.cpp
)

target_link_libraries(O_SilKitLegacyAbi
    PRIVATE SilKitInterface
    PRIVATE I_SilKit_CreateParticipantImpl
    PRIVATE I_SilKit_CreateSilKitRegistryImpl
    PRIVATE I_SilKit_VersionImpl
    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Experimental
)


set(SilKitImplObjectLibraries "")
list(APPEND SilKitImplObjectLibraries
    O_SilKit_Config
    O_SilKit_Core_Participant
    O_SilKit_Core_Service
    O_SilKit_Core_RequestReply
    O_SilKit_Core_RequestReply_ParticipantReplies
    O_SilKit_Core_VAsio
    O_SilKit_Experimental
    O_SilKit_Extensions
    O_SilKit_Experimental_NetworkSimulatorInternals
    O_SilKit_Services_Can
    O_SilKit_Services_Ethernet
    O_SilKit_Services_Flexray
    O_SilKit_Services_Lin
    O_SilKit_Services_Logging
    O_SilKit_Services_Orchestration
    O_SilKit_Services_PubSub
    O_SilKit_Services_Rpc
    O_SilKit_Tracing
    O_SilKit_Util_FileHelpers
    O_SilKit_Util_Filesystem
    O_SilKit_Util_SetThreadName
    O_SilKit_Util_SignalHandler
    O_SilKit_Util_Uuid
    O_SilKit_Util_Uri
    O_SilKit_Util_LabelMatching

    O_SilKit_Capi

    O_SilKit_CreateParticipantImpl
    O_SilKit_CreateSilKitRegistryImpl
    O_SilKit_VersionImpl

    O_SilKitLegacyAbi
)


add_library(S_SilKitImpl STATIC)

target_include_directories(S_SilKitImpl
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(S_SilKitImpl
    PUBLIC SilKitInterface
    PUBLIC Threads::Threads

    PUBLIC ${SilKitImplObjectLibraries}
)


add_library(SilKit ${silkitLibType}
    SilKitForceIncludes.cpp
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
            PDB_OUTPUT_DIRECTORY_RELEASE ${SILKIT_SYMBOLS_DIR}/Release
            PDB_OUTPUT_DIRECTORY_DEBUG ${SILKIT_SYMBOLS_DIR}/Debug
            LINK_FLAGS "/DEBUG" #make sure the resulting .dll has a .pdb file
    )
elseif(UNIX AND NOT APPLE)
    #for reproducible builds
    if(SILKIT_BUILD_REPRODUCIBLE)
        target_link_options(SilKit
        PRIVATE -Wl,--build-id=none)
    endif()
    if(SILKIT_LINK_LLD)
        target_link_options(SilKit
            PRIVATE -fuse-ld=lld)
	    if(NOT SILKIT_BUILD_REPRODUCIBLE)
		target_link_options(SilKit
		    PRIVATE -Wl,--build-id=sha1)
	    endif()
    endif()
    target_link_options(SilKit PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/silkit.map")
endif()

if ((CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang) AND NOT SILKIT_USE_SYSTEM_LIBRARIES)
    set(SILKIT_LINK_EXCLUDE_LIBS "")
    # add the file name (xxx.a) to the list
    list(APPEND SILKIT_LINK_EXCLUDE_LIBS "$<TARGET_FILE_NAME:spdlog>")
    list(APPEND SILKIT_LINK_EXCLUDE_LIBS "$<TARGET_FILE_NAME:yaml-cpp>")
    # format the list such that the linker flag accepts it
    list(JOIN SILKIT_LINK_EXCLUDE_LIBS ":" SILKIT_LINK_EXCLUDE_LIBS)
    # add the excluded libs to the linker command line
    target_link_options(SilKit PRIVATE "-Wl,--exclude-libs=${SILKIT_LINK_EXCLUDE_LIBS}")

endif()

set_target_properties(SilKit PROPERTIES
 RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 LIBRARY_OUTPUT_NAME SilKit
)

if(SILKIT_BUILD_LINUX_PACKAGE)
    set_target_properties(SilKit PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(SilKit
    PUBLIC SilKitInterface
    PUBLIC Threads::Threads

    PRIVATE ${SilKitImplObjectLibraries}
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
        CONFIGURATIONS Release Debug RelWithDebInfo
        LIBRARY
            COMPONENT bin
            DESTINATION ${INSTALL_LIB_DIR}
            NAMELINK_SKIP
            LIBRARY DESTINATION ${INSTALL_LIB_DIR} # for Linux shared object libraries (.so)
            ARCHIVE DESTINATION ${INSTALL_LIB_DIR} # for static libraries (.lib/.a)
            RUNTIME DESTINATION ${INSTALL_BIN_DIR} # for executables and dynamic libraries (.dll)
        PUBLIC_HEADER
            PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR} # Destination for header files
            COMPONENT dev
    )

    install(
        TARGETS SilKit
        EXPORT SilKitTargets
        CONFIGURATIONS Release Debug RelWithDebInfo
        LIBRARY
            COMPONENT dev
            DESTINATION ${INSTALL_LIB_DIR}
            NAMELINK_ONLY
    )
# portable installation  definitions
    install(EXPORT SilKitTargets
        DESTINATION ${INSTALL_LIB_DIR}/cmake/SilKit
        NAMESPACE "SilKit::"
        FILE SilKitTargets.cmake
        COMPONENT dev
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
    CONFIGURATIONS Release Debug RelWithDebInfo
    COMPONENT dev
)

#make our local build dir usable for other projects
export(EXPORT SilKitTargets
    NAMESPACE "SilKit::"
    FILE SilKitTargets.cmake
    )


if(SILKIT_PACKAGE_SYMBOLS)
    include(SilKitBuildTools)
    silkit_package_debugsymbols(SilKit)
endif()
