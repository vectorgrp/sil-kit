# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(SilKitSimFlexray STATIC
    FlexrayController.cpp
    FlexrayController.hpp
    FlexrayDatatypeUtils.cpp
    FlexrayDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp

    FlexraySerdes.hpp
    FlexraySerdes.cpp
)

target_include_directories(SilKitSimFlexray
    PUBLIC .
)

target_link_libraries(SilKitSimFlexray
    PRIVATE SilKitInterface
            SilKitInternal
            SilKitMwLogging
            SilKitTracing
            SilKitConfiguration
)

add_silkit_test(TestSimFlexrayValidation SOURCES ValidationTest.cpp LIBS SilKitSimFlexray)
add_silkit_test(TestSimFlexrayController
    SOURCES
        FlexrayControllerTest.cpp
        FlexrayControllerConfigTest.cpp
    LIBS 
        SilKitSimFlexray
        SilKitMwMockParticipant
        SilKitTracing
        SilKitConfiguration
        SilKitMwMockNullConnection
)
add_silkit_test(TestSimFlexray_Serdes SOURCES FlexraySerdesTest.cpp LIBS SilKitInternal SilKitSimFlexray)
