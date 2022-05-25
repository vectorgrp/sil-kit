# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimFlexray STATIC
    FlexrayControllerProxy.cpp
    FlexrayControllerProxy.hpp
    FlexrayDatatypeUtils.cpp
    FlexrayDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp

    FlexrayControllerFacade.hpp
    FlexrayControllerFacade.cpp

    FlexraySerdes.hpp
    FlexraySerdes.cpp
)

target_include_directories(IbSimFlexray
    PUBLIC .
)

target_link_libraries(IbSimFlexray
    PRIVATE IbInterface
            IbInternal
            IbMwLogging
            IbTracing
            IbConfiguration
)

add_vib_test(TestSimFlexrayValidation SOURCES ValidationTest.cpp LIBS IbSimFlexray)
add_vib_test(TestSimFlexrayController
    SOURCES
        FlexrayControllerProxyTest.cpp
        FlexrayControllerFacadeTest.cpp
    LIBS 
        IbSimFlexray
        IbMwMockParticipant
        IbTracing
        IbConfiguration
        IbMwMockNullConnection
)
add_vib_test(TestSimFlexray_Serdes SOURCES FlexraySerdesTest.cpp LIBS IbInternal IbSimFlexray)
