add_library(IbMwComAdapter STATIC
    ComAdapter.hpp
    ComAdapter_impl.hpp
    ComAdapter.cpp
)

target_link_libraries(IbMwComAdapter
    PUBLIC  IbInterface
    PUBLIC  IbMwLogging
    PUBLIC  IbMwFastRtps
    PRIVATE IbConfig
    PRIVATE IbUtil
    PRIVATE IbMwSync
    PRIVATE IbSimCan
    PRIVATE IbSimEthernet
    PRIVATE IbSimFlexRay
    PRIVATE IbSimLin
    PRIVATE IbSimIO
    PRIVATE IbSimGenericMessage
)

target_include_directories(IbMwComAdapter
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwComAdapter PRIVATE "/bigobj")
endif (MSVC)

add_mvsc_ib_include_filter()

add_vib_test(TestMwComAdapter SOURCES ComAdapterTest.cpp LIBS
    IbMwComAdapter
    IbConfig
    IbUtil
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
)