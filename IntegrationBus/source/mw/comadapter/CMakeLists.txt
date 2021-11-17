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
    PUBLIC  IbInternal
    PUBLIC  IbVersion
    PUBLIC  IbMwLogging
    PUBLIC  IbConfig
    PUBLIC  IbUtil
    PUBLIC  IbMwSync
    PUBLIC  IbSimCan
    PUBLIC  IbSimEthernet
    PUBLIC  IbSimFlexRay
    PUBLIC  IbSimLin
    PUBLIC  IbSimIO
    PUBLIC  IbSimGenericMessage
    PUBLIC  IbTracing
)

if (IB_MW_ENABLE_FASTRTPS)
    target_link_libraries(IbMwComAdapter PUBLIC  IbMwFastRtps)
endif()
if (IB_MW_ENABLE_VASIO)
    target_link_libraries(IbMwComAdapter PUBLIC  IbMwVAsio)
endif()

target_include_directories(IbMwComAdapter
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwComAdapter PRIVATE "/bigobj")
endif (MSVC)

add_vib_test(TestMwComAdapter SOURCES ComAdapterTest.cpp LIBS IbMwMockNullConnection)
