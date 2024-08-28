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

#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ParticipantConfiguration.hpp"
#include "MockParticipant.hpp"
#include "Logger.hpp"
#include "LogMsgSender.hpp"

namespace {

using namespace std::chrono_literals;

using testing::Return;
using testing::A;
using testing::An;
using testing::_;
using testing::InSequence;
using testing::NiceMock;

using namespace testing;
using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::Logging;

using SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD((void), SendMsg_LogMsg, (const IServiceEndpoint*, LogMsg));
    void SendMsg(const IServiceEndpoint* from, Services::Logging::LogMsg&& msg) override
    {
        SendMsg_LogMsg(from, msg);
    }
    void SendMsg(const IServiceEndpoint* from, const Services::Logging::LogMsg& msg) override
    {
        SendMsg_LogMsg(from, msg);
    }
};


auto ALogMsgWith(std::string loggerName, Level level, std::string payload) -> Matcher<LogMsg>
{
    return AllOf(Field(&LogMsg::loggerName, loggerName), Field(&LogMsg::level, level), Field(&LogMsg::payload, payload)

    );
}

auto ALogMsgWith(std::string loggerName, Level level, std::string payload,
                 std::unordered_map<std::string, std::string> keyValues) -> Matcher<LogMsg>
{
    return AllOf(Field(&LogMsg::loggerName, loggerName), Field(&LogMsg::level, level), Field(&LogMsg::payload, payload),
                 Field(&LogMsg::keyValues, keyValues));
}

TEST(Test_Logger, log_level_conversion)
{
    Level in{Level::Critical};
    auto lvlStr = to_string(in);
    auto out = from_string(lvlStr);
    EXPECT_EQ(in, out) << "string representation was: " << lvlStr;

    out = from_string("garbage");
    EXPECT_EQ(out, Level::Off);
}

TEST(Test_Logger, send_log_message_with_sender)
{
    ServiceDescriptor controllerAddress{"P1", "N1", "C2", 8};

    MockParticipant mockParticipant;

    LogMsgSender logMsgSender(&mockParticipant);

    logMsgSender.SetServiceDescriptor(controllerAddress);

    LogMsg msg;
    msg.loggerName = "Logger";
    msg.level = Level::Info;
    msg.payload = std::string{"some payload"};

    EXPECT_CALL(mockParticipant, SendMsg_LogMsg(&logMsgSender, msg)).Times(1);

    logMsgSender.SendLogMsg(msg);
}

TEST(Test_Logger, send_log_message_from_logger)
{
    std::string loggerName{"ParticipantAndLogger"};

    Config::Logging config;
    auto sink = Config::Sink{};
    sink.level = Level::Info;
    sink.type = Config::Sink::Type::Remote;

    config.sinks.push_back(sink);

    Logger logger{loggerName, config};

    ServiceDescriptor controllerAddress{"P1", "N1", "C2", 8};
    MockParticipant mockParticipant;
    LogMsgSender logMsgSender(&mockParticipant);
    logMsgSender.SetServiceDescriptor(controllerAddress);


    logger.RegisterRemoteLogging([&logMsgSender](LogMsg logMsg) { logMsgSender.SendLogMsg(std::move(logMsg)); });

    std::string payload{"Test log message"};

    EXPECT_CALL(mockParticipant, SendMsg_LogMsg(testing::_, ALogMsgWith(loggerName, Level::Info, payload))).Times(1);

    EXPECT_CALL(mockParticipant, SendMsg_LogMsg(testing::_, ALogMsgWith(loggerName, Level::Critical, payload)))
        .Times(1);

    logger.Info(payload);
    logger.Critical(payload);
}

TEST(Test_Logger, get_log_level)
{
    std::string loggerName{"ParticipantAndLogger"};

    Config::Logging config;
    auto sink1 = Config::Sink{};
    sink1.level = Level::Debug;
    sink1.type = Config::Sink::Type::Stdout;

    auto sink2 = Config::Sink{};
    sink2.level = Level::Critical;
    sink2.type = Config::Sink::Type::File;

    config.sinks.push_back(sink1);
    config.sinks.push_back(sink2);

    Logger logger{loggerName, config};

    // GetLogLevel should return lowest log level of all sinks
    EXPECT_EQ(logger.GetLogLevel(), Level::Debug);
}

TEST(Test_Logger, LogOnceFlag_check_setter)
{
    LogOnceFlag once;
    EXPECT_EQ(once.WasCalled(), false);
    EXPECT_EQ(once.WasCalled(), true);
    EXPECT_EQ(once.WasCalled(), true);
}


TEST(Test_Logger, send_loggermessage_from_logger)
{
    std::string loggerName{"ParticipantAndLogger"};

    Config::Logging config;
    auto sink1 = Config::Sink{};
    sink1.level = Level::Debug;
    sink1.type = Config::Sink::Type::Remote;

    config.sinks.push_back(sink1);

    Logger logger{loggerName, config};

    ServiceDescriptor controllerAddress{"P1", "N1", "C2", 8};
    MockParticipant mockParticipant;
    LogMsgSender logMsgSender(&mockParticipant);
    logMsgSender.SetServiceDescriptor(controllerAddress);

    logger.RegisterRemoteLogging([&logMsgSender](LogMsg logMsg) { logMsgSender.SendLogMsg(std::move(logMsg)); });

    std::string payload{"Test log message"};
    std::string key{"TestKey"};
    std::string value{"TestValue"};
    std::unordered_map<std::string, std::string> keyValue{{key, value}};

    EXPECT_CALL(mockParticipant,
                SendMsg_LogMsg(&logMsgSender, ALogMsgWith(loggerName, Level::Debug, payload, keyValue)))
        .Times(1);
    EXPECT_CALL(mockParticipant,
                SendMsg_LogMsg(&logMsgSender, ALogMsgWith(loggerName, Level::Critical, payload, keyValue)))
        .Times(1);

    LoggerMessage lm{&logger, Level::Debug};
    lm.SetMessage(payload);
    lm.SetKeyValue(key, value);
    lm.Dispatch();

    LoggerMessage lm2{&logger, Level::Critical};
    lm2.SetMessage(payload);
    lm2.SetKeyValue(key, value);
    lm2.Dispatch();
}

} // anonymous namespace
