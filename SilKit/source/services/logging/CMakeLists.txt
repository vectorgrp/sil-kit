# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
    PRIVATE spdlog
)

if(UNIX)
    set_target_properties(O_SilKit_Services_Logging
        PROPERTIES
        VISIBILITY_INLINES_HIDDEN 1
        CXX_VISIBILITY_PRESET hidden
    )
endif()


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_Logger.cpp 
    LIBS S_SilKitImpl I_SilKit
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_LoggingSerdes.cpp LIBS S_SilKitImpl)
