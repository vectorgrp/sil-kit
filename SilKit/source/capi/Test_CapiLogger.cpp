// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/logging/all.hpp"

namespace {
using namespace SilKit::Services::Logging;

class MockLogger : public SilKit::Services::Logging::ILogger
{
public:
    MOCK_METHOD2(Log, void(Level, const std::string&));
    MOCK_METHOD1(Trace, void(const std::string&));
    MOCK_METHOD1(Debug, void(const std::string&));
    MOCK_METHOD1(Info, void(const std::string&));
    MOCK_METHOD1(Warn, void(const std::string&));
    MOCK_METHOD1(Error, void(const std::string&));
    MOCK_METHOD1(Critical, void(const std::string&));

    MOCK_CONST_METHOD0(GetLogLevel, Level());
};

class Test_CapiLogger : public testing::Test
{
public:
    MockLogger mockLogger;
    Test_CapiLogger() {}
};

TEST_F(Test_CapiLogger, logger_function_mapping)
{
    SilKit_ReturnCode returnCode;

    EXPECT_CALL(mockLogger, Log(Level::Off, "Test message")).Times(testing::Exactly(1));
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

    EXPECT_CALL(mockLogger, Log(Level::Off, "Test message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Off, "Test message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Trace, "Trace message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Trace, "Trace message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Debug, "Debug message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Debug, "Debug message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Info, "Info message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Info, "Info message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Warn, "Warn message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Warn, "Warn message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Error, "Error message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Error, "Error message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, Log(Level::Critical, "Critical message")).Times(testing::Exactly(1));
    returnCode = SilKit_Logger_Log((SilKit_Logger*)&mockLogger, SilKit_LoggingLevel_Critical, "Critical message");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockLogger, GetLogLevel()).Times(testing::Exactly(1));
    SilKit_LoggingLevel logLevel;
    returnCode = SilKit_Logger_GetLogLevel((SilKit_Logger*)&mockLogger, &logLevel);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}
} // namespace
