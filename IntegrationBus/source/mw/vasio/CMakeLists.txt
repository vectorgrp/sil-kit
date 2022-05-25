# Copyright (c) Vector Informatik GmbH. All rights reserved.
find_package(Threads REQUIRED)

add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp


    VAsioProtocolVersion.hpp
    VAsioSerdes.hpp
    VAsioSerdes.cpp

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

    SerializedMessageTraits.hpp
    SerializedMessage.hpp
    SerializedMessage.cpp
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

add_vib_test(Test_VAsioSerdes SOURCES VAsioSerdesTest.cpp LIBS IbMwVAsio)
