add_library(IbSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    EthControllerProxy.cpp
    EthControllerProxy.hpp
    EthDatatypeUtils.cpp
    EthDatatypeUtils.hpp
    EthFrame.cpp
    EthFrame.hpp
    PcapTracer.cpp
    PcapTracer.hpp
    NamedPipe.hpp
)
if(WIN32)
    target_sources(IbSimEthernet PRIVATE
        NamedPipeWin.hpp
        NamedPipeWin.cpp
        )
elseif(UNIX)
    target_sources(IbSimEthernet PRIVATE
        NamedPipeLinux.hpp
        NamedPipeLinux.cpp
        )
else()
    message(FATAL_ERROR "ERROR: unsupported platform for NamedPipe!")
endif()

target_link_libraries(IbSimEthernet
    PRIVATE IbInterface
)

target_include_directories(IbSimEthernet
    PUBLIC .
)

add_vib_test(TestSimEthController SOURCES EthControllerTest.cpp EthControllerProxyTest.cpp LIBS IbSimEthernet IbMwMockComAdapter)
