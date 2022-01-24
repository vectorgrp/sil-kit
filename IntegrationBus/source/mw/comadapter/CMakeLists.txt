# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwComAdapter STATIC
    ComAdapter.hpp
    ComAdapter_impl.hpp
    ComAdapter.cpp
    CreateComAdapter.hpp
    CreateComAdapter.cpp
)

target_link_libraries(IbMwComAdapter
    INTERFACE  IbInterface
    PUBLIC
        IbInternal
        IbMwLogging
        IbConfig_deprecated
        IbUtil
        IbSetThreadName
        IbUuid
        IbMwSync
        IbSimCan
        IbSimEthernet
        IbSimFlexRay
        IbSimLin
        IbSimIO
        IbSimGenericMessage
        IbSimDataMessage
        IbSimRpc
        IbTracing
        IbMwService
    INTERFACE  IbMwService
    PRIVATE
        IbVersion
)

if (IB_MW_ENABLE_VASIO)
    target_link_libraries(IbMwComAdapter PUBLIC IbMwVAsio)
endif()

target_include_directories(IbMwComAdapter
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwComAdapter PRIVATE "/bigobj")
endif (MSVC)

add_vib_test(TestMwComAdapter 
    SOURCES ComAdapterTest.cpp 
    LIBS IbMwMockNullConnection
)
