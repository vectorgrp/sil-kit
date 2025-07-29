# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Logging INTERFACE)

target_include_directories(I_SilKit_Services_Logging
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Logging
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Config
    INTERFACE I_SilKit_Core_Service
    INTERFACE I_SilKit_Core_RequestReply
)

################################################################################
# Thirdparty -- Logging
################################################################################
# We use fmtlib to offer a convenient logging interface

if (TARGET ${SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET})
    message(STATUS "have fmt-header-only")
    target_link_libraries(I_SilKit_Services_Logging
        INTERFACE $<BUILD_INTERFACE:${SILKIT_THIRD_PARTY_FMT_HEADER_ONLY_TARGET}>
    )
    target_include_directories(I_SilKit_Services_Logging INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(I_SilKit_Services_Logging
        INTERFACE $<BUILD_INTERFACE:HAVE_FMTLIB>
        INTERFACE $<BUILD_INTERFACE:FMT_HEADER_ONLY>
    )
endif()


add_library(O_SilKit_Services_Logging OBJECT
    SpdlogTypeConversion.hpp
    IMsgForLogMsgSender.hpp
    IMsgForLogMsgReceiver.hpp

    LogMsgSender.hpp
    LogMsgSender.cpp

    LogMsgReceiver.hpp
    LogMsgReceiver.cpp

    ILoggerInternal.hpp
    Logger.hpp
    Logger.cpp
    #string formatting for SIL Kit types
    SilKitFmtFormatters.hpp
    LoggerMessage.hpp
    LogFunctions.hpp
    LoggingSerdes.hpp
    LoggingSerdes.cpp
    StructuredLoggingKeys.hpp
)

target_link_libraries(O_SilKit_Services_Logging
    PUBLIC I_SilKit_Services_Logging
    PRIVATE I_SilKit_Util_StringHelpers
    PRIVATE spdlog
)

if(UNIX)
    set_target_properties(O_SilKit_Services_Logging
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()


if (SILKIT_BUILD_TESTS)
    add_library(I_SilKit_Services_Logging_Testing INTERFACE
        MockLogger.hpp
    )

    target_include_directories(I_SilKit_Services_Logging_Testing
        INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif ()


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_Logger.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_LoggingSerdes.cpp LIBS S_SilKitImpl)

