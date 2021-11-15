# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_subdirectory(idl)

add_library(IbMwFastRtps STATIC
    IbSubListener.hpp
    FastRtpsConnection.hpp
    FastRtpsConnection.cpp
    FastRtpsUtils.hpp
    FastRtpsUtils.cpp
    fastrtps_fwd.h

    IdlTraitMacro.hpp
    IdlTraitsCan.hpp
    IdlTraitsEth.hpp
    IdlTraitsFlexray.hpp
    IdlTraitsIo.hpp
    IdlTraitsLin.hpp
    IdlTraitsLogging.hpp
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
    ReportMatchingListener.hpp
)

target_link_libraries(IbMwFastRtps
    PUBLIC IbUtil
    PUBLIC IbInterface
    PUBLIC fastrtps
    PUBLIC fastcdr
    IbMwFastRtpsIdl
    PUBLIC IbMwLogging
    INTERFACE IbInternal
)

target_include_directories(IbMwFastRtps
    PUBLIC .
)

if (MSVC)
    target_compile_options(IbMwFastRtps PRIVATE "/bigobj")
endif (MSVC)
