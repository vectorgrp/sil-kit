# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimFlexRay STATIC
    FrController.cpp
    FrController.hpp
    FrControllerProxy.cpp
    FrControllerProxy.hpp
    FrDatatypeUtils.cpp
    FrDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp
)

target_include_directories(IbSimFlexRay
    PUBLIC .
)

target_link_libraries(IbSimFlexRay
    PRIVATE IbInterface
            IbMwLogging
            IbTracing
)

add_vib_test(TestSimFrValidation SOURCES ValidationTest.cpp LIBS IbSimFlexRay)
add_vib_test(TestSimFrController 
    SOURCES 
        FrControllerTest.cpp 
        FrControllerProxyTest.cpp 
    LIBS 
        IbSimFlexRay
        IbMwMockComAdapter
        IbTracing
)
