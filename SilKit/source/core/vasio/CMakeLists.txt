# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

find_package(Threads REQUIRED)

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

    IPeerMetrics.hpp
    PeerMetrics.hpp
    PeerMetrics.cpp
)

target_link_libraries(O_SilKit_Core_VAsio
    PRIVATE I_SilKit
)

target_compile_definitions(O_SilKit_Core_VAsio
    PRIVATE ASIO_STANDALONE
)

target_include_directories(O_SilKit_Core_VAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/io
)

if (MSVC)
    target_compile_options(O_SilKit_Core_VAsio PRIVATE "/bigobj")
endif()

if(MINGW)
    target_link_libraries(O_SilKit_Core_VAsio PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()


add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioConnection.cpp LIBS S_SilKitImpl I_SilKit)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioRegistry.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioSerdes.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_SerializedMessage.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TransformAcceptorUris.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_VAsioCapabilities.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_RingBuffer.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES io/Test_IoContext.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES io/Test_AsioIoContext.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests SOURCES io/util/Test_TracingMacrosDetails.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ConnectPeer.cpp LIBS S_SilKitImpl I_SilKit)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ConnectKnownParticipants.cpp LIBS S_SilKitImpl I_SilKit)

# Testing interoperability between different protocol versions requires testing on a higher level:
# We instantiate a complete Participant<VAsioConnection> with a specific version
# and do integration tests here
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ParticipantVersion.cpp LIBS S_SilKitImpl S_ITests_STH)
