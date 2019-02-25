add_library(IbSimLin STATIC
    LinController.cpp
    LinController.hpp
    LinControllerProxy.cpp
    LinControllerProxy.hpp
    LinDatatypeUtils.cpp
    LinDatatypeUtils.hpp
)

target_include_directories(IbSimLin
    PUBLIC .
)

target_link_libraries(IbSimLin
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()

make_gtest(TestSimLinControllerProxy LinControllerProxyTest.cpp IbSimLin IbMwMock)
make_gtest(TestSimLinController LinControllerTest.cpp IbSimLin IbMwMock)
