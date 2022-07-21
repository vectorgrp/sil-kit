# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimCan STATIC
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp
    CanController.cpp
    CanController.hpp
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    CanSerdes.hpp
    CanSerdes.cpp
)

target_include_directories(SilKitSimCan
    PUBLIC .
)

target_link_libraries(SilKitSimCan
    PUBLIC  SilKitWireCan
    PRIVATE SilKitInterface
            SilKitInternal
            SilKitTracing
            SilKitConfiguration
    INTERFACE SilKitInternal
            SilKitConfiguration
)

add_silkit_test(TestSimCanController
    SOURCES 
        CanControllerConfigTest.cpp
        CanControllerDetailedSimTest.cpp 
        CanControllerTrivialSimTest.cpp
    LIBS 
        SilKitSimCan
        SilKitMwMockParticipant
        SilKitTracing
        SilKitMwMockNullConnection
)

add_silkit_test(TestSimCan_Serdes  SOURCES CanSerdesTest.cpp LIBS SilKitInternal SilKitSimCan)

add_silkit_test(TestSimCan_StringUtils SOURCES CanStringUtilsTest.cpp LIBS SilKitInternal SilKitSimCan)
