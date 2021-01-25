add_library(IbSimLin STATIC
    LinController.cpp
    LinController.hpp

    LinControllerProxy.cpp
    LinControllerProxy.hpp

    LinControllerReplay.cpp
    LinControllerReplay.hpp
)

target_include_directories(IbSimLin
    PUBLIC .
)

target_link_libraries(IbSimLin
    PRIVATE IbInterface
            IbMwLogging
            IbTracing
)

add_vib_test(TestSimLin
    SOURCES
        LinTestUtils.hpp
        LinControllerTest.cpp
        LinControllerProxyTest.cpp
    LIBS
        IbSimLin
        IbMwMockComAdapter
        IbTracing
)
