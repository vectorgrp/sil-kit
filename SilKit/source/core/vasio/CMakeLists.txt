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

find_package(Threads REQUIRED)


add_library(I_SilKit_Core_VAsio INTERFACE)

target_include_directories(I_SilKit_Core_VAsio
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/io
)

target_link_libraries(I_SilKit_Core_VAsio
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Config
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Core_Service
    INTERFACE I_SilKit_Core_RequestReply
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_Logging
    INTERFACE I_SilKit_Services_Metrics
    INTERFACE I_SilKit_Services_Orchestration
    INTERFACE I_SilKit_Services_PubSub
    INTERFACE I_SilKit_Services_Rpc
    INTERFACE I_SilKit_Util
    INTERFACE I_SilKit_Util_Filesystem
    INTERFACE I_SilKit_Util_Uri

    INTERFACE ${SILKIT_THIRD_PARTY_ASIO}
    INTERFACE Threads::Threads
)


add_library(O_SilKit_Core_VAsio OBJECT
    VAsioConnection.hpp
    VAsioConnection.cpp

    VAsioRegistry.hpp
    VAsioRegistry.cpp

    IConnectionMethods.hpp

    VAsioProtocolVersion.hpp
    VAsioSerdes.hpp
    VAsioSerdes.cpp
    VAsioSerdes_Protocol30.hpp
    VAsioSerdes_Protocol30.cpp

    SilKitLink.hpp
    VAsioDatatypes.hpp
    VAsioMsgKind.hpp
    VAsioPeerInfo.hpp
    VAsioReceiver.hpp
    VAsioTransmitter.hpp

    IVAsioPeer.hpp

    VAsioPeer.hpp
    VAsioPeer.cpp
    VAsioProxyPeer.hpp
    VAsioProxyPeer.cpp

    TransformAcceptorUris.hpp
    TransformAcceptorUris.cpp

    SerializedMessageTraits.hpp
    SerializedMessage.hpp
    SerializedMessage.cpp

    VAsioCapabilities.hpp
    VAsioCapabilities.cpp

    io/impl/AsioCleanupEndpoint.cpp
    io/impl/AsioFormatEndpoint.cpp
    io/impl/AsioGenericRawByteStream.cpp
    io/impl/AsioIoContext.cpp
    io/impl/AsioTimer.cpp
    io/impl/SetAsioSocketOptions.cpp
    io/MakeAsioIoContext.cpp

    ConnectPeer.cpp
    ConnectKnownParticipants.cpp
    RemoteConnectionManager.cpp

    RingBuffer.hpp
    RingBuffer.cpp
)

target_link_libraries(O_SilKit_Core_VAsio
    PUBLIC I_SilKit_Core_VAsio
)

target_compile_definitions(O_SilKit_Core_VAsio PRIVATE ASIO_STANDALONE)

target_include_directories(O_SilKit_Core_VAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/io
)


if (SILKIT_BUILD_TESTS)
    add_library(I_SilKit_Core_VAsio_Testing INTERFACE
        io/mock/MockAcceptor.hpp
        io/mock/MockConnector.hpp
        io/mock/MockIoContext.hpp
        io/mock/MockRawByteStream.hpp
        io/mock/MockTimer.hpp
        mock/MockConnectionMethods.hpp
        mock/MockConnectKnownParticipantsListener.hpp
        mock/MockConnectPeer.hpp
        mock/MockVAsioPeer.hpp
    )

    target_include_directories(I_SilKit_Core_VAsio_Testing
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/io/mock
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mock
    )

    target_link_libraries(I_SilKit_Core_VAsio_Testing
        INTERFACE I_SilKit_Core_VAsio
    )
endif ()


if (MSVC)
    target_compile_options(O_SilKit_Core_VAsio PRIVATE "/bigobj")
    target_compile_definitions(I_SilKit_Core_VAsio INTERFACE _WIN32_WINNT=0x0601)
endif()
if(MINGW)
    target_compile_definitions(I_SilKit_Core_VAsio INTERFACE _WIN32_WINNT=0x0601)
    target_link_libraries(O_SilKit_Core_VAsio PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioConnection.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioRegistry.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioSerdes.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_SerializedMessage.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TransformAcceptorUris.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioCapabilities.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_RingBuffer.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES io/Test_IoContext.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES io/Test_AsioIoContext.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES io/util/Test_TracingMacrosDetails.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ConnectPeer.cpp LIBS S_SilKitImpl I_SilKit_Services_Logging_Testing I_SilKit_Core_VAsio_Testing)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ConnectKnownParticipants.cpp LIBS S_SilKitImpl I_SilKit_Services_Logging_Testing I_SilKit_Core_VAsio_Testing)

# Testing interoperability between different protocol versions requires testing on a higher level:
# We instantiate a complete Participant<VAsioConnection> with a specific version
# and do integration tests here
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ParticipantVersion.cpp LIBS S_SilKitImpl S_ITests_STH)
