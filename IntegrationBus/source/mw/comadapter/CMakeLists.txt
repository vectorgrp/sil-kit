add_library(IbMwComAdapter STATIC
    ComAdapter.hpp
    ComAdapter_impl.hpp
    ComAdapter.cpp
    IComAdapter_internal.hpp
    CreateComAdapter.hpp
    CreateComAdapter.cpp
)

target_link_libraries(IbMwComAdapter
    PUBLIC  IbInterface
    PUBLIC  IbMwLogging
    PUBLIC  IbMwFastRtps
    PUBLIC  IbMwVAsio
    PUBLIC  IbConfig
    PUBLIC  IbUtil
    PUBLIC  IbMwSync
    PUBLIC  IbSimCan
    PUBLIC  IbSimEthernet
    PUBLIC  IbSimFlexRay
    PUBLIC  IbSimLin
    PUBLIC  IbSimIO
    PUBLIC  IbSimGenericMessage
)

target_include_directories(IbMwComAdapter
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwComAdapter PRIVATE "/bigobj")
endif (MSVC)

add_vib_test(TestMwComAdapter SOURCES ComAdapterTest.cpp LIBS IbMwMockNullConnection)
