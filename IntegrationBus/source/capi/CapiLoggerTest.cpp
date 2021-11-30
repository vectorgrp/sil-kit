// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/logging/all.hpp"

namespace {
    using namespace ib::mw::logging;

    class MockLogger : public ib::mw::logging::ILogger
    {
    public:
        MOCK_METHOD2(Log, void(Level, const std::string&));
        MOCK_METHOD1(Trace, void(const std::string&));
        MOCK_METHOD1(Debug, void(const std::string&));
        MOCK_METHOD1(Info, void(const std::string&));
        MOCK_METHOD1(Warn, void(const std::string&));
        MOCK_METHOD1(Error, void(const std::string&));
        MOCK_METHOD1(Critical, void(const std::string&));

        MOCK_CONST_METHOD1(ShouldLog, bool(Level level));
    };

    class CapiLoggerTest : public testing::Test
    {
    public:
        MockLogger mockLogger;
        CapiLoggerTest()
        {
        }
    };

    TEST_F(CapiLoggerTest, logger_function_mapping)
    {
        ib_ReturnCode returnCode;

        EXPECT_CALL(mockLogger, Log(Level::Off, "Test message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Off, "Test message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

    TEST_F(CapiLoggerTest, logger_nullpointer_params)
    {
        ib_ReturnCode returnCode;

        returnCode = ib_Logger_Log(nullptr, ib_LoggingLevel_Off, "Test message");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Off, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

    TEST_F(CapiLoggerTest, logger_enum_mappings)
    {
        ib_ReturnCode returnCode;

        EXPECT_CALL(mockLogger, Log(Level::Off, "Test message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Off, "Test message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Trace, "Trace message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Trace, "Trace message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Debug, "Debug message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Debug, "Debug message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Info, "Info message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Info, "Info message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Warn, "Warn message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Warn, "Warn message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Error, "Error message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Error, "Error message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockLogger, Log(Level::Critical, "Critical message")).Times(testing::Exactly(1));
        returnCode = ib_Logger_Log((ib_Logger*)&mockLogger, ib_LoggingLevel_Critical, "Critical message");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }
}
