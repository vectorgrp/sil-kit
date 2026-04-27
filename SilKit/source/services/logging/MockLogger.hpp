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


inline auto ALoggerMessageWith(SilKit::Services::Logging::Level level, std::string payload)
    -> testing::Matcher<const SilKit::Services::Logging::LoggerMessage&>
{
    return testing::AllOf(
        testing::Property(&SilKit::Services::Logging::LoggerMessage::GetLevel, testing::Eq(level)),
        testing::Property(&SilKit::Services::Logging::LoggerMessage::GetMsgString, testing::HasSubstr(payload)));
}

inline auto ALoggerMessageWith(SilKit::Services::Logging::Level level)
    -> testing::Matcher<const SilKit::Services::Logging::LoggerMessage&>
{
    return testing::Property(&SilKit::Services::Logging::LoggerMessage::GetLevel, testing::Eq(level));
}


class MockLogger : public ::SilKit::Services::Logging::ILoggerInternal
{
    using Level = ::SilKit::Services::Logging::Level;
    using LoggerMessage = ::SilKit::Services::Logging::LoggerMessage;
    using LogMsg = ::SilKit::Services::Logging::LogMsg;
    using Topic = ::SilKit::Services::Logging::Topic;

public:
    MockLogger()
    {
        ON_CALL(*this, GetLogLevel).WillByDefault(testing::Return(Level::Trace));
        // Topic filtering for Log methods
        /* ON_CALL(*this, Log(testing::_, testing::_, testing::_))
            .WillByDefault(
            [this](Level level, Topic topic, const std::string& msg) {
                if (!IsTopicEnabled(topic)) return;
                LogImpl(level, topic, msg);
            });*/
    }

    std::vector<Topic> disabledTopics;
    std::vector<Topic> EnabledTopics;

    SilKit::Services::Logging::ILogger* AsILogger() override
    {
        throw SilKit::SilKitError("Not implemented!");
    }

    bool IsTopicEnabled(Topic topic) const
    {

        if (!disabledTopics.empty())
        {
            return std::find(disabledTopics.begin(), disabledTopics.end(), topic) == disabledTopics.end();
        }
        return true;
    }

    void LogImpl(Level , Topic , const std::string& )
    {
        // Call the mock method for test verification
        // This is just for demonstration, actual test code will use EXPECT_CALL
    }

    void Log(Level lvl, Topic topic, const std::string& msg) override
    {
        this->MakeMessage(lvl, topic).SetMessage(msg).Dispatch();
    }

public:
    MOCK_METHOD(void, ProcessLoggerMessage, (const LoggerMessage& msg), (override));
    MOCK_METHOD(void, LogReceivedMsg, (const LogMsg& msg), (override));

    LoggerMessage MakeMessage(Level level, Topic topic) override
    {
        return LoggerMessage(this, level, topic);
    }
    MOCK_METHOD(SilKit::Services::Logging::Level, GetLogLevel, (), (const, override));
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
