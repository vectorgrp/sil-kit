# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimEthernet STATIC
    EthController.cpp
    EthController.hpp
    #EthControllerReplay.hpp
    #EthControllerReplay.cpp
    
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

target_link_libraries(SilKitSimEthernet
    PUBLIC
        SilKitTracing
        SilKitWireEthernet
    PRIVATE
        SilKitInternal
        SilKitInterface
        SilKitMwLogging
)

target_include_directories(SilKitSimEthernet
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_silkit_test(TestSimEthController
    SOURCES 
        EthControllerTrivialSimTest.cpp
        EthControllerDetailedSimTest.cpp
        EthControllerConfigTest.cpp
    LIBS
        SilKitSimEthernet
        SilKitMwMockParticipant
        SilKitTracing
        SilKitMwMockNullConnection
)
add_silkit_test(TestSimEthernet_Serdes  SOURCES EthernetSerdesTest.cpp LIBS SilKitSimEthernet)
