# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


add_library(I_SilKit_Services_Rpc INTERFACE)

target_include_directories(I_SilKit_Services_Rpc
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)


add_library(S_SilKit_Services_Rpc STATIC
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

target_link_libraries(S_SilKit_Services_Rpc
    PUBLIC I_SilKit_Services_Rpc

    PRIVATE SilKitInterface
            I_SilKit_Core_Internal
            S_SilKit_Util_Uuid
            I_SilKit_Config
            S_SilKit_Util_LabelMatching
)

add_silkit_test(Test_SimRpc
    SOURCES
        Test_RpcClient.cpp
        Test_RpcServer.cpp
        Test_RpcMatching.cpp
    LIBS
        S_SilKit_Services_Rpc
        I_SilKit_Core_Mock_Participant
        I_SilKit_Core_Internal
        S_SilKit_Core_Mock_NullConnection
        S_SilKit_Config
)
add_silkit_test(Test_RpcSerdes SOURCES Test_RpcSerdes.cpp LIBS I_SilKit_Core_Internal S_SilKit_Services_Rpc)
