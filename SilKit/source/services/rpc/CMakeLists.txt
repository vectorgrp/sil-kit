# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Services_Rpc OBJECT
    RpcCallHandle.hpp
    RpcDatatypeUtils.hpp
    RpcDatatypeUtils.cpp
    RpcServer.hpp
    RpcServer.cpp
    RpcClient.hpp
    RpcClient.cpp
    RpcServerInternal.hpp
    RpcServerInternal.cpp

    RpcSerdes.hpp
    RpcSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Rpc
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        Test_RpcClient.cpp
        Test_RpcServer.cpp
        Test_RpcMatching.cpp
    LIBS
        S_SilKitImpl
        I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_RpcSerdes.cpp LIBS S_SilKitImpl I_SilKit)
