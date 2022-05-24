# Copyright (c) Vector Informatik GmbH. All rights reserved.
find_package(Threads REQUIRED)

add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    VAsioProtocol.hpp
    VAsioProtocol.cpp
    SerdesCompat.hpp
    SerdesCompat.cpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesMwVAsio.hpp
    SerdesMwService.hpp
    SerdesSimData.hpp
    SerdesSimRpc.hpp
    IbLink.hpp
    IVAsioPeer.hpp
    VAsioDatatypes.hpp
    VAsioMsgKind.hpp
    VAsioPeerInfo.hpp
    VAsioReceiver.hpp
    VAsioRegistry.hpp
    VAsioRegistry.cpp
    VAsioTcpPeer.hpp
    VAsioTcpPeer.cpp
    VAsioTransmitter.hpp
    Uri.hpp
    Uri.cpp
)

target_link_libraries(IbMwVAsio
    PUBLIC
      IbUtil
      IbInterface
      IbInternal
      IbMwSync
      IbMwLogging
      IbMwService
      IbSimDataMessage
      IbSimRpc
      IbSimCan
      IbSimEthernet
      IbSimLin
      IbSimFlexray
      Threads::Threads
      IbFilesystem
      asio
    INTERFACE
      IbConfiguration
)

target_compile_definitions(IbMwVAsio PRIVATE ASIO_STANDALONE)

target_include_directories(IbMwVAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)


if (MSVC)
    target_compile_options(IbMwVAsio PRIVATE "/bigobj")
    target_compile_definitions(IbMwVAsio PUBLIC _WIN32_WINNT=0x0601)
endif()
if(MINGW)
    target_compile_definitions(IbMwVAsio PUBLIC _WIN32_WINNT=0x0601)
    target_link_libraries(IbMwVAsio PUBLIC -lwsock32 -lws2_32) #windows socket/ wsa
endif()

add_vib_test(TestMwVAsioConnection SOURCES VAsioConnectionTest.cpp LIBS IbMwVAsio IbMwMockParticipant)

# XXX linking against IbInternal is required because VAsioProtocolVersion.hpp pulls in EndpointAddress.hpp indirectly
add_vib_test(TestMwVAsio_SerdesMw SOURCES SerdesMwTest.cpp LIBS IbInternal)
add_vib_test(TestMwVAsio_SerdesMwSync SOURCES SerdesMwSyncTest.cpp LIBS IbMwSync)
add_vib_test(TestMwVAsio_SerdesSimData SOURCES SerdesSimDataTest.cpp LIBS IbInternal)
add_vib_test(TestMwVAsio_SerdesMwService SOURCES SerdesMwServiceTest.cpp LIBS IbMwService)
add_vib_test(TestMwVAsio_SerdesMwVAsio SOURCES SerdesMwVAsioTest.cpp LIBS IbMwVAsio)
