# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimCan STATIC
    CanController.cpp
    CanController.hpp
    CanControllerProxy.cpp
    CanControllerProxy.hpp
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp

    CanControllerFacade.cpp
    CanControllerFacade.hpp
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
        CanControllerTest.cpp 
        CanControllerProxyTest.cpp 
        CanControllerFacadeTest.cpp
    LIBS 
        IbSimCan 
        IbMwMockComAdapter
        IbTracing
        IbMwMockNullConnection
)
