// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/cfg/ConfigBuilder.hpp"

#include "MockComAdapter.hpp"

#include "Logger.hpp"
#include "LogMsgDistributor.hpp"
#include "LogMsgReceiver.hpp"

#include "spdlog/sinks/sink.h"

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
using ib::mw::test::DummyLogger;

class MockComAdapter : public DummyComAdapter
{
public:
    void SendIbMessage(EndpointAddress from, LogMsg&& msg)
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const LogMsg&));
    MOCK_METHOD2(SendIbMessage_proxy, void(EndpointAddress, const LogMsg&));
};

class MockLogger : public DummyLogger
{
public:
    MOCK_METHOD1(LogReceivedMsg, void(const LogMsg&));
};

auto ALogMsgWith(std::string logger_name, Level level, std::string payload) -> Matcher<const LogMsg&>
{
    return AllOf(
        Field(&LogMsg::logger_name, logger_name),
        Field(&LogMsg::level, level),
        Field(&LogMsg::payload, payload)
    );
}

TEST(LoggerTest, send_log_message_with_distributor)
{
    EndpointAddress controllerAddress = {3, 8};

    MockComAdapter mockComAdapter;

    LogMsgDistributor logMsgDistributor(&mockComAdapter);
    logMsgDistributor.SetEndpointAddress(controllerAddress);

    LogMsg msg;
    msg.logger_name = "Logger";
    msg.level = Level::info;
    msg.payload = std::string{"some payload"};

    EXPECT_CALL(mockComAdapter, SendIbMessage(controllerAddress, msg))
        .Times(1);

    logMsgDistributor.SendLogMsg(msg);
}

TEST(LoggerTest, receive_log_message_with_receiver)
{
    EndpointAddress senderAddress{17, 4};

    MockComAdapter mockComAdapter;
    MockLogger mockLogger;

    LogMsgReceiver logMsgReceiver(&mockComAdapter);
    logMsgReceiver.SetLogger(&mockLogger);

    LogMsg msg;

    EXPECT_CALL(mockLogger, LogReceivedMsg(msg))
        .Times(1);

    logMsgReceiver.ReceiveIbMessage(senderAddress, msg);
}

TEST(LoggerTest, create_logger_sinks)
{
    cfg::ConfigBuilder configBuilder("TestBuilder");

    configBuilder.SimulationSetup().AddParticipant("L1")
        ->AddLogger(cfg::Logger::Type::Remote, logging::Level::warn)
        ->AddLogger(cfg::Logger::Type::File, logging::Level::trace)
        .WithFilename("FileLogger");

    auto config = configBuilder.Build();

    auto&& participantConfig = cfg::get_by_name(config.simulationSetup.participants, "L1");
    Logger logger{"L1", participantConfig.logger};

    ASSERT_EQ(logger.GetSinks().size(), 3u);
    auto&& sinks = logger.GetSinks();
    
    EXPECT_EQ(sinks[1]->level(), spdlog::level::warn);
    EXPECT_EQ(sinks[2]->level(), spdlog::level::trace);
}

TEST(LoggerTest, send_log_message_from_logger)
{
    std::string loggerName{"ParticipantAndLogger"};

    cfg::ConfigBuilder configBuilder("TestBuilder");
    configBuilder.SimulationSetup().AddParticipant(loggerName)
        ->AddLogger(cfg::Logger::Type::Remote, logging::Level::debug);

    auto config = configBuilder.Build();
    auto&& participantConfig = cfg::get_by_name(config.simulationSetup.participants, loggerName);
    Logger logger{loggerName, participantConfig.logger};

    EndpointAddress controllerAddress = {3, 8};
    MockComAdapter mockComAdapter;
    LogMsgDistributor logMsgDistributor(&mockComAdapter);
    logMsgDistributor.SetEndpointAddress(controllerAddress);

    logger.RegisterLogMsgHandler([&logMsgDistributor](logging::LogMsg logMsg) {

        logMsgDistributor.SendLogMsg(std::move(logMsg));

    });

    std::string payload{"Test log message"};

    EXPECT_CALL(mockComAdapter, SendIbMessage_proxy(controllerAddress,
        ALogMsgWith(loggerName, Level::info, payload)))
        .Times(1);

    logger.Info(payload);

    EXPECT_CALL(mockComAdapter, SendIbMessage_proxy(controllerAddress,
        ALogMsgWith(loggerName, Level::critical, payload)))
        .Times(1);

    logger.Critical(payload);
}

}  // anonymous namespace
