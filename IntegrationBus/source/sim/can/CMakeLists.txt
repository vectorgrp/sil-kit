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

make_gtest(TestSimCanControllerProxy CanControllerProxyTest.cpp IbSimCan IbMwMock)
make_gtest(TestSimCanController CanControllerTest.cpp IbSimCan IbMwMock)
