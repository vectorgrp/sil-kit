add_library(IbSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    EthControllerProxy.cpp
    EthControllerProxy.hpp
    EthDatatypeUtils.cpp
    EthDatatypeUtils.hpp
    EthFrame.cpp
    EthFrame.hpp
)

target_include_directories(IbSimEthernet
    PUBLIC .
)

target_link_libraries(IbSimEthernet
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()
