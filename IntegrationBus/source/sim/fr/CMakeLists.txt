# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimFlexRay STATIC
    FrControllerProxy.cpp
    FrControllerProxy.hpp
    FrDatatypeUtils.cpp
    FrDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp

    FrControllerFacade.hpp
    FrControllerFacade.cpp
)

target_include_directories(IbSimFlexRay
    PUBLIC .
)

target_link_libraries(IbSimFlexRay
    PRIVATE IbInterface
            IbInternal
            IbMwLogging
            IbTracing
            IbConfiguration
)

add_vib_test(TestSimFrValidation SOURCES ValidationTest.cpp LIBS IbSimFlexRay)
add_vib_test(TestSimFrController 
    SOURCES 
        FrControllerProxyTest.cpp 
    LIBS 
        IbSimFlexRay
        IbMwMockComAdapter
        IbTracing
        IbConfiguration
)
