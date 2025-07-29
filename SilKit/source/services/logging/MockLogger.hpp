// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "LoggerMessage.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Services {
namespace Logging {


class MockLogger : public ::SilKit::Services::Logging::ILoggerInternal
{
    using Level = ::SilKit::Services::Logging::Level;
    using LoggerMessage = ::SilKit::Services::Logging::LoggerMessage;
    using LogMsg = ::SilKit::Services::Logging::LogMsg;

public:
    MockLogger()
    {
        ON_CALL(*this, GetLogLevel).WillByDefault(testing::Return(Level::Trace));
    }

public:
    MOCK_METHOD(void, Log, (Level level, const std::string& msg), (override));
    MOCK_METHOD(void, ProcessLoggerMessage, (const LoggerMessage& msg), (override));
    MOCK_METHOD(void, LogReceivedMsg, (const LogMsg& msg), (override));

    void Trace(const std::string& msg) override
    {
        Log(Level::Trace, msg);
    }

    void Debug(const std::string& msg) override
    {
        Log(Level::Debug, msg);
    }

    void Info(const std::string& msg) override
    {
        Log(Level::Info, msg);
    }

    void Warn(const std::string& msg) override
    {
        Log(Level::Warn, msg);
    }

    void Error(const std::string& msg) override
    {
        Log(Level::Error, msg);
    }

    void Critical(const std::string& msg) override
    {
        Log(Level::Critical, msg);
    }

    MOCK_METHOD(SilKit::Services::Logging::Level, GetLogLevel, (), (const, override));
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
