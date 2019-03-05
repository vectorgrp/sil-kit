add_subdirectory(idl)

add_library(IbMwFastRtps STATIC
    IbSubListener.hpp
    FastRtpsComAdapter.hpp
    FastRtpsComAdapter.cpp
    FastRtpsGuard.hpp
    FastRtpsGuard.cpp
    fastrtps_fwd.h
    IdlTraitMacro.hpp
    IdlTraitsCan.hpp
    IdlTraitsEth.hpp
    IdlTraitsFlexray.hpp
    IdlTraitsIo.hpp
    IdlTraitsLin.hpp
    IdlTraitsSync.hpp
    IdlTypeConversionCan.hpp
    IdlTypeConversionEth.hpp
    IdlTypeConversionFlexray.hpp
    IdlTypeConversionGenericMsg.hpp
    IdlTypeConversionLin.hpp
    IdlTypeConversionIo.hpp
    IdlTypeConversionMw.hpp
    IdlTypeConversionSync.hpp
    IdlTypeConversionLogging.hpp
    IdlTypeConversionLogging_impl.hpp
    memory_fastrtps.hpp
    ReportMatchingListener.hpp
)

target_link_libraries(IbMwFastRtps
    PRIVATE IbInterface
    IbUtil
    IbMwFastRtpsIdl
    IbMwLogging
    IbMwSync
    IbSimCan
    IbSimEthernet
    IbSimFlexRay
    IbSimLin
    IbSimIO
    IbSimGenericMessage
    fastrtps
    fastcdr
)

target_include_directories(IbMwFastRtps
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwFastRtps PRIVATE "/bigobj")
endif (MSVC)

add_mvsc_ib_include_filter()
