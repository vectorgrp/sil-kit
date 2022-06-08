# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    EthControllerReplay.hpp
    EthControllerReplay.cpp
    
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp
    
    EthernetSerdes.hpp
    EthernetSerdes.cpp
)

target_link_libraries(IbSimEthernet
    PUBLIC
        IbTracing
    PRIVATE
        IbInternal
        IbInterface
        IbMwLogging
)

target_include_directories(IbSimEthernet
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_vib_test(TestSimEthController
    SOURCES 
        EthControllerTrivialSimTest.cpp
        EthControllerDetailedSimTest.cpp
        EthControllerConfigTest.cpp
    LIBS
        IbSimEthernet
        IbMwMockParticipant
        IbTracing
        IbMwMockNullConnection
)
add_vib_test(TestSimEthernet_Serdes  SOURCES EthernetSerdesTest.cpp LIBS IbSimEthernet)
