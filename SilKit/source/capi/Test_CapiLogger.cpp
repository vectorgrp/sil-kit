/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
        Test_CapiLogger()
        {
        }
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
}
