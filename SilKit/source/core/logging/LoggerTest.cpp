// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

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
using namespace SilKit::Core::Logging;

using SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD((void), SendMsg, (const IServiceEndpoint*, LogMsg&&));
};

auto ALogMsgWith(std::string logger_name, Level level, std::string payload) -> Matcher<LogMsg&&>
{
    return AllOf(
        Field(&LogMsg::logger_name, logger_name),
        Field(&LogMsg::level, level),
        Field(&LogMsg::payload, payload)
    );
}

TEST(LoggerTest, log_level_conversion)
{
    Level in{Level::Critical};
    auto lvlStr = to_string(in);
    auto out = from_string(lvlStr);
    EXPECT_EQ(in, out) << "string representation was: " << lvlStr;

    out = from_string("garbage");
    EXPECT_EQ(out, Level::Off);
}

TEST(LoggerTest, send_log_message_with_sender)
{
    EndpointAddress controllerAddress = {3, 8};

    MockParticipant mockParticipant;

    LogMsgSender logMsgSender(&mockParticipant);
    
    logMsgSender.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    LogMsg msg;
    msg.logger_name = "Logger";
    msg.level = Level::Info;
    msg.payload = std::string{"some payload"};

    EXPECT_CALL(mockParticipant, SendMsg(&logMsgSender, std::move(msg)))
        .Times(1);

    logMsgSender.SendLogMsg(std::move(msg));
}

TEST(LoggerTest, send_log_message_from_logger)
{
    std::string loggerName{"ParticipantAndLogger"};

    Config::Logging config;
    auto sink = Config::Sink{};
    sink.level = SilKit::Core::Logging::Level::Debug;
    sink.type = Config::Sink::Type::Remote;

    config.sinks.push_back(sink);

    Logger logger{loggerName, config};

    EndpointAddress controllerAddress = {3, 8};
    MockParticipant mockParticipant;
    LogMsgSender logMsgSender(&mockParticipant);
    logMsgSender.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    logger.RegisterRemoteLogging([&logMsgSender](Logging::LogMsg logMsg) {

        logMsgSender.SendLogMsg(std::move(logMsg));

    });

    std::string payload{"Test log message"};

    EXPECT_CALL(mockParticipant, SendMsg(&logMsgSender,
        ALogMsgWith(loggerName, Level::Info, payload)))
        .Times(1);

    logger.Info(payload);

    EXPECT_CALL(mockParticipant, SendMsg(&logMsgSender,
        ALogMsgWith(loggerName, Level::Critical, payload)))
        .Times(1);

    logger.Critical(payload);
}

}  // anonymous namespace
