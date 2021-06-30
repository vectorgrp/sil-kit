find_package(Threads REQUIRED)

add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    MessageBuffer.hpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesMwVAsio.hpp
    SerdesSimGeneric.hpp
    SerdesSimCan.hpp
    SerdesSimEthernet.hpp
    SerdesSimIo.hpp
    SerdesSimLin.hpp
    SerdesSimFlexray.hpp
    IbMsgTraits.hpp
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
      IbSimGenericMessage
      IbSimCan
      IbSimEthernet
      IbSimIO    
      IbSimLin
      IbSimFlexRay
      Threads::Threads
      IbFilesystem
      asio
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
add_vib_test(TestMwVAsio_SerdesMwSync SOURCES SerdesMwSyncTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimCan SOURCES SerdesSimCanTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimEthernet SOURCES SerdesSimEthernetTest.cpp LIBS IbSimEthernet)
add_vib_test(TestMwVAsio_SerdesSimLin SOURCES SerdesSimLinTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimFlexray SOURCES SerdesSimFlexrayTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimGeneric SOURCES SerdesSimGenericTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimIo SOURCES SerdesSimIoTest.cpp)
