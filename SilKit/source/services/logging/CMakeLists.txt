# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


add_library(SilKitMwLogging STATIC
    SpdlogTypeConversion.hpp
    IMsgForLogMsgSender.hpp
    IMsgForLogMsgReceiver.hpp

    LogMsgSender.hpp
    LogMsgSender.cpp

    LogMsgReceiver.hpp
    LogMsgReceiver.cpp

    ILogger.hpp
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
    PUBLIC SilKitConfiguration
    PUBLIC SilKitMwService

    PRIVATE spdlog
)

if(UNIX)
    set_target_properties(SilKitMwLogging
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()

add_silkit_test(Test_MwLoggingLogger 
    SOURCES Test_Logger.cpp 
    LIBS SilKitMwLogging SilKitMwMockParticipant
)
add_silkit_test(Test_MwLogging_Serdes SOURCES Test_LoggingSerdes.cpp LIBS SilKitMwLogging )
