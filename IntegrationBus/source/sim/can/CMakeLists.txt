# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbSimCan STATIC
    CanController.cpp
    CanController.hpp
    CanControllerProxy.cpp
    CanControllerProxy.hpp
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp

    CanControllerReplay.cpp
    CanControllerReplay.hpp
)

target_include_directories(IbSimCan
    PUBLIC .
)

target_link_libraries(IbSimCan
    PRIVATE IbInterface
            IbTracing
)

add_vib_test(TestSimCanController
    SOURCES 
        CanControllerTest.cpp 
        CanControllerProxyTest.cpp 
    LIBS 
        IbSimCan 
        IbMwMockComAdapter
        IbTracing
)
set_target_properties(TestSimCanController PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
