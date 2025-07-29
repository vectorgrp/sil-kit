# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Rpc INTERFACE)

target_include_directories(I_SilKit_Services_Rpc
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Rpc
    INTERFACE SilKitInterface
)


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
    PUBLIC I_SilKit_Services_Rpc

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Util_Uuid
    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Util_LabelMatching
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        Test_RpcClient.cpp
        Test_RpcServer.cpp
        Test_RpcMatching.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        I_SilKit_Core_Internal
        I_SilKit_Core_Mock_NullConnection
        I_SilKit_Config
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_RpcSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
