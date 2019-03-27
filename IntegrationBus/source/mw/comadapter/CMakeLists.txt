add_library(IbMwComAdapter STATIC
    ComAdapter.hpp
    ComAdapter_impl.hpp
    ComAdapter.cpp
)

target_link_libraries(IbMwComAdapter
    PUBLIC  IbInterface
    PUBLIC  IbMwLogging
    PUBLIC  IbMwFastRtps
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
