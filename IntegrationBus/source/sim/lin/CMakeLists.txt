# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimLin STATIC
    LinController.cpp
    LinController.hpp
    
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp

    LinSerdes.hpp
    LinSerdes.cpp
)

target_include_directories(IbSimLin
    PUBLIC .
)

target_link_libraries(IbSimLin
    PUBLIC  IbInterface
    PRIVATE IbInterface
            IbInternal
            IbMwLogging
            IbTracing
)

add_vib_test(TestSimLin
    SOURCES
        LinTestUtils.hpp
        LinControllerDetailedSimTest.cpp
        LinControllerTrivialSimTest.cpp
        LinControllerConfigTest.cpp
    LIBS
        IbSimLin
        IbMwMockParticipant
        IbTracing
        IbMwMockNullConnection
)
add_vib_test(TestSimLin_Serdes  SOURCES LinSerdesTest.cpp LIBS IbInternal IbSimLin)
