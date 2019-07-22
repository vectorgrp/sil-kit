add_library(IbSimIO STATIC
    InPort.hpp
    OutPort.hpp
    IoDatatypeUtils.cpp
    IoDatatypeUtils.hpp
)

target_include_directories(IbSimIO
    PUBLIC .
)

target_link_libraries(IbSimIO
    PRIVATE IbInterface
)

add_vib_test(TestSimInPort SOURCES InPortTest.cpp LIBS IbSimIO IbMwMock)
add_vib_test(TestSimOutPort SOURCES OutPortTest.cpp LIBS IbSimIO IbMwMock)
