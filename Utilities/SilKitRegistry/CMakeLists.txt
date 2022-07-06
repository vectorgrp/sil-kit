# Copyright (c) Vector Informatik GmbH. All rights reserved.
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

include(SilKitInstall)

if(MSVC)
    #disable warnings on visual studio 2015 v14
    if(MSVC_VERSION EQUAL 1900)
        add_compile_options("/wd4503")
    endif()
endif (MSVC)

add_executable(sil-kit-registry
    SignalHandler.hpp
    SignalHandler.cpp
    Registry.cpp
)

# Group this demo project into a folder
set_target_properties(sil-kit-registry PROPERTIES
     FOLDER "Utilities"
     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

# we're linking statically with SilKit components and not with the dll.
add_definitions(-DEXPORT_SilKitAPI)

target_link_libraries(sil-kit-registry
    SilKitMwVAsio
    SilKitConfiguration
    SilKitVersion
)
install(TARGETS sil-kit-registry
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)
