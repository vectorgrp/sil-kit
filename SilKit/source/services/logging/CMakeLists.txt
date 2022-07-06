# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitMwLogging STATIC
    SpdlogTypeConversion.hpp
    IMsgForLogMsgSender.hpp
    IMsgForLogMsgReceiver.hpp

    LogMsgSender.hpp
    LogMsgSender.cpp

    LogMsgReceiver.hpp
    LogMsgReceiver.cpp

    Logger.hpp
    Logger.cpp
    #string formatting for SIL Kit types
    SilKitFmtFormatters.hpp

    LoggingSerdes.hpp
    LoggingSerdes.cpp
)
################################################################################
# Thirdparty -- Logging
################################################################################
# We use fmtlib to offer a convenient logging interface

if (TARGET fmt-header-only)
    message(STATUS "have fmt-header-only")
    target_link_libraries(SilKitMwLogging
        INTERFACE
            $<BUILD_INTERFACE:fmt-header-only>
        PRIVATE
            $<BUILD_INTERFACE:fmt-header-only>
    )
    target_compile_definitions(SilKitMwLogging
        INTERFACE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
        PRIVATE
            $<BUILD_INTERFACE:HAVE_FMTLIB>
            $<BUILD_INTERFACE:FMT_HEADER_ONLY>
    )
endif()

target_include_directories(SilKitMwLogging
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SilKitMwLogging
    PUBLIC SilKitInterface
    PUBLIC SilKitInternal
    PRIVATE spdlog
    PRIVATE SilKitMwSync
    PUBLIC SilKitMwService
    INTERFACE SilKitConfiguration
    PRIVATE SilKitConfiguration
)

if(UNIX)
    set_target_properties(SilKitMwLogging
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()

add_silkit_test(TestMwLoggingLogger 
    SOURCES LoggerTest.cpp 
    LIBS SilKitMwLogging SilKitMwMockParticipant
)
add_silkit_test(TestMwLogging_Serdes SOURCES LoggingSerdesTest.cpp LIBS SilKitMwLogging )
