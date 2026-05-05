// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/logging/all.hpp"
#include "LoggingTopics.hpp"
#include "MockLogger.hpp"

namespace {
using namespace SilKit::Services::Logging;

class Test_CapiLogger : public testing::Test
{
public:
    MockLogger mockLogger;
    Test_CapiLogger() {}
};

TEST_F(Test_CapiLogger, logger_function_mapping)
{
    SilKit_ReturnCode returnCode;

    EXPECT_CALL(mockLogger, ProcessLoggerMessage(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Off, "Test message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiLogger, logger_nullpointer_params)
{
    SilKit_ReturnCode returnCode;

    returnCode = SilKit_Logger_Log(nullptr, SilKit_LoggingLevel_Off, "Test message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Off, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiLogger, logger_enum_mappings)
{
    SilKit_ReturnCode returnCode;

    // SilKit_Logger_Log dispatches via MakeMessage → ProcessLoggerMessage
    EXPECT_CALL(mockLogger, ProcessLoggerMessage(testing::_)).Times(testing::Exactly(7));

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Off, "Test message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Trace, "Trace message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Debug, "Debug message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Info, "Info message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Warn, "Warn message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Error, "Error message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Critical, "Critical message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, GetLogLevel()).Times(testing::Exactly(1));
    SilKit_LoggingLevel logLevel;
    returnCode = SilKit_Logger_GetLogLevel((SilKit_Logger*)&mockLogger, &logLevel);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}
} // namespace
