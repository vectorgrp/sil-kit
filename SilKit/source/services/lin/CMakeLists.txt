# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimLin STATIC
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

target_include_directories(SilKitSimLin
    PUBLIC .
)

target_link_libraries(SilKitSimLin
    PUBLIC  SilKitInterface
            SilKitWireLin
    PRIVATE SilKitInterface
            SilKitInternal
            SilKitMwLogging
            SilKitTracing
)

add_silkit_test(TestSimLin
    SOURCES
        LinTestUtils.hpp
        LinControllerDetailedSimTest.cpp
        LinControllerTrivialSimTest.cpp
        LinControllerConfigTest.cpp
    LIBS
        SilKitSimLin
        SilKitMwMockParticipant
        SilKitTracing
        SilKitMwMockNullConnection
)
add_silkit_test(TestSimLin_Serdes  SOURCES LinSerdesTest.cpp LIBS SilKitInternal SilKitSimLin)
