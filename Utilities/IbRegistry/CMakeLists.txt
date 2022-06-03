# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
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
