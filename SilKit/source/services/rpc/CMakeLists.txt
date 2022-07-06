# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimRpc STATIC
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

target_include_directories(SilKitSimRpc
    PUBLIC .
)

target_link_libraries(SilKitSimRpc
    PRIVATE SilKitInterface
            SilKitInternal
            SilKitUuid
            SilKitConfiguration
)

add_silkit_test(TestSimRpc
    SOURCES
        RpcClientTest.cpp
        RpcServerTest.cpp
    LIBS
        SilKitSimRpc
        SilKitMwMockParticipant
        SilKitInternal
        SilKitMwMockNullConnection
)
add_silkit_test(TestSimRpc_Serdes SOURCES RpcSerdesTest.cpp LIBS SilKitInternal SilKitSimRpc)
