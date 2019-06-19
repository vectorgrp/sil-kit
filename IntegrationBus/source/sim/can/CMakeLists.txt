add_library(IbSimCan STATIC
    CanController.cpp
    CanController.hpp
    CanControllerProxy.cpp
    CanControllerProxy.hpp
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp
)

target_include_directories(IbSimCan
    PUBLIC .
)

target_link_libraries(IbSimCan
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()

add_vib_test(TestSimCanControllerProxy SOURCES CanControllerProxyTest.cpp LIBS IbSimCan IbMwMock)
add_vib_test(TestSimCanController SOURCES CanControllerTest.cpp LIBS IbSimCan IbMwMock)
