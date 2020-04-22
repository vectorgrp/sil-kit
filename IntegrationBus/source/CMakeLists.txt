add_subdirectory(cfg)
add_subdirectory(mw)
add_subdirectory(sim)
add_subdirectory(util)
add_subdirectory(extensions)

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

# Published extension implementations follow here for the extension loading
# side of the code. The actual shared library code might reside 
# anywhere.

target_sources(IntegrationBus
    PRIVATE
    #for IDE:
    ${CMAKE_CURRENT_LIST_DIR}/../include/ib/extensions/IIbRegistry.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../include/ib/extensions/CreateExtension.hpp
    # implementations
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
endif (MSVC)

 set_target_properties(IntegrationBus PROPERTIES
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
 )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(IntegrationBus
    PUBLIC  IbInterface
    PRIVATE IbConfig
    PRIVATE IbMwComAdapter
    PRIVATE IbExtensions
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
