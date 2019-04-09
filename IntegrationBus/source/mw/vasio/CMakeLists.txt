add_library(IbMwVAsio STATIC
    VAsioConnection.hpp
    VAsioConnection.cpp
    MessageBuffer.hpp
    SerdesMw.hpp
    SerdesMwSync.hpp
    SerdesSimCan.hpp
)

target_link_libraries(IbMwVAsio
    PUBLIC IbUtil
    PUBLIC IbInterface
)

target_include_directories(IbMwVAsio
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwVAsio PRIVATE "/bigobj")
endif (MSVC)

add_mvsc_ib_include_filter()

add_vib_test(TestMwVAsio_MessageBuffer SOURCES MessageBufferTest.cpp)
add_vib_test(TestMwVAsio_SerdesMw SOURCES SerdesMwTest.cpp)
add_vib_test(TestMwVAsio_SerdesMwSync SOURCES SerdesMwSyncTest.cpp)
add_vib_test(TestMwVAsio_SerdesSimCan SOURCES SerdesSimCanTest.cpp)
