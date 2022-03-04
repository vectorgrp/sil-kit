// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ParticipantConfiguration.hpp"

#include "MockComAdapter.hpp"

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
using namespace ib;
using namespace ib::mw;
using namespace ib::mw::logging;

using ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD((void), SendIbMessage, (const IIbServiceEndpoint*, LogMsg&&));
};

auto ALogMsgWith(std::string logger_name, Level level, std::string payload) -> Matcher<LogMsg&&>
{
    return AllOf(
        Field(&LogMsg::logger_name, logger_name),
        Field(&LogMsg::level, level),
        Field(&LogMsg::payload, payload)
    );
}

TEST(LoggerTest, send_log_message_with_sender)
{
    EndpointAddress controllerAddress = {3, 8};

    MockComAdapter mockComAdapter;

    LogMsgSender logMsgSender(&mockComAdapter);
    
    logMsgSender.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    LogMsg msg;
    msg.logger_name = "Logger";
    msg.level = Level::Info;
    msg.payload = std::string{"some payload"};

    EXPECT_CALL(mockComAdapter, SendIbMessage(&logMsgSender, std::move(msg)))
        .Times(1);

    logMsgSender.SendLogMsg(std::move(msg));
}

TEST(LoggerTest, send_log_message_from_logger)
{
    std::string loggerName{"ParticipantAndLogger"};

    cfg::v1::datatypes::Logging config;
    auto sink = cfg::v1::datatypes::Sink{};
    sink.level = ib::mw::logging::Level::Debug;
    sink.type = cfg::v1::datatypes::Sink::Type::Remote;

    config.sinks.push_back(sink);

    Logger logger{loggerName, config};

    EndpointAddress controllerAddress = {3, 8};
    MockComAdapter mockComAdapter;
    LogMsgSender logMsgSender(&mockComAdapter);
    logMsgSender.SetServiceDescriptor(from_endpointAddress(controllerAddress));

    logger.RegisterRemoteLogging([&logMsgSender](logging::LogMsg logMsg) {

        logMsgSender.SendLogMsg(std::move(logMsg));

    });

    std::string payload{"Test log message"};

    EXPECT_CALL(mockComAdapter, SendIbMessage(&logMsgSender,
        ALogMsgWith(loggerName, Level::Info, payload)))
        .Times(1);

    logger.Info(payload);

    EXPECT_CALL(mockComAdapter, SendIbMessage(&logMsgSender,
        ALogMsgWith(loggerName, Level::Critical, payload)))
        .Times(1);

    logger.Critical(payload);
}

}  // anonymous namespace
