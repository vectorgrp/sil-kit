# Copyright (c) Vector Informatik GmbH. All rights reserved.
find_package(Threads REQUIRED)

add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    MessageBuffer.hpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesMwVAsio.hpp
    SerdesMwService.hpp
    SerdesSimData.hpp
    SerdesSimRpc.hpp
    SerdesSimCan.hpp
    SerdesSimEthernet.hpp
    SerdesSimLin.hpp
    SerdesSimFlexray.hpp
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
      IbMwSync
      IbMwLogging
      IbMwService
      IbSimDataMessage
      IbSimRpc
      IbSimCan
      IbSimEthernet
      IbSimLin
      IbSimFlexRay
      Threads::Threads
      IbFilesystem
      asio
    INTERFACE
      IbInternal
      IbConfiguration
)

target_compile_definitions(IbMwVAsio PRIVATE ASIO_STANDALONE)

target_include_directories(IbMwVAsio
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)


if (MSVC)
    target_compile_options(IbMwVAsio PRIVATE "/bigobj")
    target_compile_definitions(IbMwVAsio PUBLIC _WIN32_WINNT=0x0601)
endif (MSVC)

add_vib_test(TestMwVAsio_MessageBuffer SOURCES MessageBufferTest.cpp)
add_vib_test(TestMwVAsio_SerdesMw SOURCES SerdesMwTest.cpp)
add_vib_test(TestMwVAsio_SerdesMwSync SOURCES SerdesMwSyncTest.cpp LIBS IbMwSync)
add_vib_test(TestMwVAsio_SerdesSimCan SOURCES SerdesSimCanTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimEthernet SOURCES SerdesSimEthernetTest.cpp LIBS IbSimEthernet)
add_vib_test(TestMwVAsio_SerdesSimLin SOURCES SerdesSimLinTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimFlexray SOURCES SerdesSimFlexrayTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimData SOURCES SerdesSimDataTest.cpp)
add_vib_test(TestMwVAsio_SerdesMwService SOURCES SerdesMwServiceTest.cpp LIBS IbMwService)
