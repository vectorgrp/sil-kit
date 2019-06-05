add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    MessageBuffer.hpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesSimCan.hpp
    SerdesSimGeneric.hpp

    IVAsioPeer.hpp
    VAsioTcpPeer.hpp
    VAsioTcpPeer.cpp
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
    PUBLIC IbSimCan
    PUBLIC IbSimGenericMessage
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
add_vib_test(TestMwVAsio_SerdesSimGeneric SOURCES SerdesSimGenericTest.cpp)
add_vib_test(TestMwVAsioConnection SOURCES VAsioConnectionTest.cpp LIBS IbMwVAsio IbMwComAdapter IbConfig gmock)
