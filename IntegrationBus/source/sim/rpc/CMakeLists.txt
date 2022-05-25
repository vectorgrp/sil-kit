# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimRpc STATIC
    RpcCallHandle.hpp
    RpcDatatypeUtils.hpp
    RpcDatatypeUtils.cpp
    RpcServer.hpp
    RpcServer.cpp
    RpcClient.hpp
    RpcClient.cpp
    RpcServerInternal.hpp
    RpcServerInternal.cpp
    RpcDiscoverer.hpp
    RpcDiscoverer.cpp

    RpcSerdes.hpp
    RpcSerdes.cpp
)

target_include_directories(IbSimRpc
    PUBLIC .
)

target_link_libraries(IbSimRpc
    PRIVATE IbInterface
            IbInternal
            IbUuid
            IbConfiguration
)

add_vib_test(TestSimRpc
    SOURCES
        RpcClientTest.cpp
        RpcServerTest.cpp
    LIBS
        IbSimRpc
        IbMwMockParticipant
        IbInternal
        IbMwMockNullConnection
)
add_vib_test(TestSimRpc_Serdes SOURCES RpcSerdesTest.cpp LIBS IbInternal IbSimRpc)
