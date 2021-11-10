# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    EthControllerProxy.cpp
    EthControllerProxy.hpp
    EthDatatypeUtils.cpp
    EthDatatypeUtils.hpp
    EthFrame.cpp
    EthFrame.hpp
    EthControllerReplay.hpp
    EthControllerReplay.cpp
)

target_link_libraries(IbSimEthernet
    PRIVATE IbInterface
    PRIVATE IbMwLogging
    PRIVATE IbTracing
)

target_include_directories(IbSimEthernet
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_vib_test(TestSimEthController
    SOURCES 
        EthControllerTest.cpp
        EthControllerProxyTest.cpp
    LIBS
        IbSimEthernet
        IbMwMockComAdapter
        IbTracing
)
