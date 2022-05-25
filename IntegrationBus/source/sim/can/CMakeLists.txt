# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimCan STATIC
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

target_include_directories(IbSimCan
    PUBLIC .
)

target_link_libraries(IbSimCan
    PRIVATE IbInterface
            IbInternal
            IbTracing
            IbConfiguration
    INTERFACE IbInternal
            IbConfiguration
)

add_vib_test(TestSimCanController
    SOURCES 
        #CanControllerTest.cpp 
        #CanControllerProxyTest.cpp 
        CanControllerConfigTest.cpp
        CanControllerDetailedSimTest.cpp 
        CanControllerTrivialSimTest.cpp
    LIBS 
        IbSimCan 
        IbMwMockParticipant
        IbTracing
        IbMwMockNullConnection
)

add_vib_test(TestSimCan_Serdes  SOURCES CanSerdesTest.cpp LIBS IbInternal IbSimCan)
