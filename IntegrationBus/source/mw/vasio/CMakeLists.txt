add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    MessageBuffer.hpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesSimGeneric.hpp
    SerdesSimCan.hpp
    SerdesSimEthernet.hpp
    SerdesSimIo.hpp
    SerdesMwRegistry.hpp
    SerdesSimLin.hpp
    SerdesSimFlexray.hpp

    IVAsioPeer.hpp
    VAsioTcpPeer.hpp
    VAsioTcpPeer.cpp
    VAsioPeerInfo.hpp
    VAsioReceiver.hpp
    VAsioSender.hpp
    VAsioMsgKind.hpp
    VAsioMessageSubscriber.hpp
)

target_link_libraries(IbMwVAsio
    PUBLIC IbUtil
    PUBLIC IbInterface
    PUBLIC IbMwSync
    PUBLIC IbMwLogging
    PUBLIC IbSimGenericMessage
    PUBLIC IbSimCan
    PUBLIC IbSimEthernet
    PUBLIC IbSimIO    
    PUBLIC IbSimLin
    PUBLIC IbSimFlexRay
)

target_include_directories(IbMwVAsio
    PUBLIC .
    PUBLIC ${THIRDPARTY_DIR}/Fast-RTPS/thirdparty/asio/asio/include/
)

target_compile_definitions(IbMwVAsio
    PUBLIC ASIO_STANDALONE
)


if (MSVC)
    target_compile_options(IbMwVAsio PRIVATE "/bigobj")
    target_compile_definitions(IbMwVAsio PUBLIC _WIN32_WINNT=0x0601)
endif (MSVC)

add_mvsc_ib_include_filter()

add_vib_test(TestMwVAsio_MessageBuffer SOURCES MessageBufferTest.cpp)
add_vib_test(TestMwVAsio_SerdesMw SOURCES SerdesMwTest.cpp)
add_vib_test(TestMwVAsio_SerdesMwSync SOURCES SerdesMwSyncTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimCan SOURCES SerdesSimCanTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimEthernet SOURCES SerdesSimEthernetTest.cpp LIBS IbSimEthernet)
add_vib_test(TestMwVAsio_SerdesSimLin SOURCES SerdesSimLinTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimFlexray SOURCES SerdesSimFlexrayTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimGeneric SOURCES SerdesSimGenericTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimIo SOURCES SerdesSimIoTest.cpp)
