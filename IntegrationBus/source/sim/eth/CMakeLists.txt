add_library(IbSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    EthControllerProxy.cpp
    EthControllerProxy.hpp
    EthDatatypeUtils.cpp
    EthDatatypeUtils.hpp
    EthFrame.cpp
    EthFrame.hpp
    EthPcapTracer.cpp
    EthPcapTracer.hpp
    NamedPipeWin.cpp
    NamedPipe.hpp
)

target_link_libraries(IbSimEthernet
    PRIVATE IbInterface
)

target_include_directories(IbSimEthernet
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbSimEthernet PRIVATE "/bigobj")
    target_compile_definitions(IbSimEthernet PUBLIC _WIN32_WINNT=0x0601)
endif (MSVC)

add_vib_test(TestSimEthController SOURCES EthControllerTest.cpp EthControllerProxyTest.cpp LIBS IbSimEthernet IbMwMockComAdapter)
