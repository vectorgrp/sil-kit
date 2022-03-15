# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
project("IntegrationBus-Registry" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

include(IntegrationBusInstall)

if(MSVC)
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()
endif (MSVC)

add_executable(IbRegistry
    SignalHandler.hpp
    SignalHandler.cpp
    IbRegistry.cpp
)

# Group this demo project into a folder
set_target_properties(IbRegistry PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# we're linking statically with IB components and not with the dll.
add_definitions(-DEXPORT_IntegrationBusAPI)

target_link_libraries(IbRegistry
    IbMwVAsio
    IbConfiguration
    IbVersion
)
install(TARGETS IbRegistry
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)


## Non-redistributable shared library version of IbRegistry app:
add_library(vib-registry 
    SHARED
    IbRegistryLib.cpp
)
target_link_libraries(vib-registry
    PRIVATE
        IbConfiguration
        IbMwVAsio
        IbVersion
        IbExtensions
)

set_target_properties(vib-registry PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
     ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

add_vib_test(
    TestIbRegistryLib
    SOURCES
        IbRegistryLibTests.cpp
    LIBS
        IbExtensions
        IbMwComAdapter
        IbFilesystem
        IntegrationBus
)

if(${IB_BUILD_TESTS})
    target_include_directories(TestIbRegistryLib
        PRIVATE ../../IntegrationBus/IntegrationTests
    )
    # NB we need this test to reside in a different directory
    # on windoes the 'GetProcessPath' is in the search path for DLLs.
    # Otherwise we cannot test the extension search path in the configuration
    set_target_properties(TestIbRegistryLib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    set_tests_properties(TestIbRegistryLib PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    )   
endif()

if(MSVC)
    #make sure we archive the PDBs for the shared lib, too
    target_compile_options(vib-registry PUBLIC /Zi )
    set_target_properties(vib-registry
        PROPERTIES
            PDB_NAME vib-registry
            PDB_OUTPUT_DIRECTORY_RELEASE ${PKG_PDB_DIR}/Release
            PDB_OUTPUT_DIRECTORY_DEBUG ${PKG_PDB_DIR}/Debug
            LINK_FLAGS "/DEBUG" #make sure the resulting .dll has a .pdb file
    )
    
endif()


#make sure we don't ship .lib files on Windows
if(MSVC)
    set(_dest "RUNTIME")
else()
    set(_dest "LIBRARY")
endif()

install(
    TARGETS vib-registry
    ${_dest} DESTINATION ${INSTALL_NONREDIST_DIR}
)

install(
    FILES Readme-NonRedistributable.txt
    DESTINATION ${INSTALL_NONREDIST_DIR}
    RENAME README.txt
)
