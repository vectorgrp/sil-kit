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
)

target_include_directories(IbSimRpc
    PUBLIC .
)

target_link_libraries(IbSimRpc
    PRIVATE IbInterface
            IbInternal
            IbUuid
    INTERFACE IbInternal
)