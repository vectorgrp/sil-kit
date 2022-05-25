# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbMwLogging STATIC
    SpdlogTypeConversion.hpp
    IIbToLogMsgSender.hpp
    IIbToLogMsgReceiver.hpp

    LogMsgSender.hpp
    LogMsgSender.cpp

    LogMsgReceiver.hpp
    LogMsgReceiver.cpp

    Logger.hpp
    Logger.cpp
    #string formatting for VIB types
    VibFmtFormatters.hpp

    LoggingSerdes.hpp
    LoggingSerdes.cpp
)
################################################################################
# Thirdparty -- Logging
################################################################################
# We use fmtlib to offer a convenient logging interface

if (TARGET fmt-header-only)
    message(STATUS "have fmt-header-only")
    target_link_libraries(IbMwLogging
        INTERFACE
            $<BUILD_INTERFACE:fmt-header-only>
        PRIVATE
            $<BUILD_INTERFACE:fmt-header-only>
    )
    target_compile_definitions(IbMwLogging
        INTERFACE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
        PRIVATE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
    )
endif()

target_include_directories(IbMwLogging
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(IbMwLogging
    PUBLIC IbInterface
    PUBLIC IbInternal
    PRIVATE spdlog
    PRIVATE IbMwSync
    PUBLIC IbMwService
    INTERFACE IbConfiguration
    PRIVATE IbConfiguration
)

if(UNIX)
    set_target_properties(IbMwLogging
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()

add_vib_test(TestMwLoggingLogger 
    SOURCES LoggerTest.cpp 
    LIBS IbMwLogging IbMwMockParticipant
)
add_vib_test(Test_LoggingSerdes SOURCES LoggingSerdesTest.cpp LIBS IbMwLogging )
