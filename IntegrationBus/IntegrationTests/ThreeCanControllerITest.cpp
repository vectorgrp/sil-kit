// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::sim::can;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

auto AnAckWithTxIdAndCanId(CanTxId transmitId, uint32_t canId) -> testing::Matcher<const CanTransmitAcknowledge&>
{
    using namespace testing;
    return AllOf(
        Field(&CanTransmitAcknowledge::transmitId, transmitId),
        Field(&CanTransmitAcknowledge::canId, canId)
    );
}

class ThreeCanControllerITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const CanTransmitAcknowledge&));
    };

protected:
    ThreeCanControllerITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());

        testMessages.resize(5);
        for (auto index = 0; index < testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            testMessages[index].expectedData = messageBuilder.str();
        }

        ib::cfg::ConfigBuilder builder{"TestConfig"};

        builder.SimulationSetup()
            .AddParticipant("CanWriter")
            .AddCan("CAN1").WithLink("CAN1");
        builder.SimulationSetup()
            .AddParticipant("CanReader1")
            .AddCan("CAN1").WithLink("CAN1");
        builder.SimulationSetup()
            .AddParticipant("CanReader2")
            .AddCan("CAN1").WithLink("CAN1");

        ibConfig = builder.Build();
    }

    void Sender()
    {
        auto comAdapter = CreateComAdapterImpl(ibConfig, "CanWriter");
        comAdapter->joinIbDomain(domainId);

        std::promise<void> threadFinishedPromise{};
        unsigned int receiveCount{0};

        auto* controller = comAdapter->CreateCanController("CAN1");
        controller->RegisterTransmitStatusHandler(
            [this, &threadFinishedPromise, &receiveCount](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);

            if (testMessages.size() == ++receiveCount)
            {
                allReceivedPromise.set_value();
                threadFinishedPromise.set_value();
            }
        });

        for (auto&& message : testMessages)
        {
            CanMessage msg;
            msg.canId = 1;
            msg.dataField.assign(message.expectedData.begin(), message.expectedData.end());
            msg.dlc = msg.dataField.size();

            std::this_thread::sleep_for(100ms);
            controller->SendMessage(std::move(msg));
        }

        auto threadFinished = threadFinishedPromise.get_future();
        ASSERT_EQ(threadFinished.wait_for(15s), std::future_status::ready);
    }

    void Receiver(const std::string& participantName, const std::shared_future<void>& finishedFuture)
    {
        auto comAdapter = CreateComAdapterImpl(ibConfig, participantName);
        comAdapter->joinIbDomain(domainId);

        unsigned int receiveCount{0};

        auto* controller = comAdapter->CreateCanController("CAN1");
        controller->RegisterTransmitStatusHandler(
            [this](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);
        });

        controller->RegisterReceiveMessageHandler(
            [this, &receiveCount, &participantName](ICanController* /*ctrl*/, const CanMessage& msg) {

            if (participantName == "CanReader1")
            {
                std::string message(msg.dataField.begin(), msg.dataField.end());
                testMessages[receiveCount++].receivedData = message;
            }
        });

        ASSERT_EQ(finishedFuture.wait_for(15s), std::future_status::ready);
    }

    void ExecuteTest()
    {
        std::shared_future<void> finishedFuture(allReceivedPromise.get_future());
        std::thread receiver1Thread{[this, &finishedFuture] { Receiver("CanReader1", finishedFuture); }};
        std::thread receiver2Thread{[this, &finishedFuture] { Receiver("CanReader2", finishedFuture); }};

        for (auto index = 1; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(index, 1))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(0, 1))).Times(0);
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(6, 1))).Times(0);

        std::thread senderThread{[this] { Sender(); }};

        senderThread.join();
        receiver1Thread.join();
        receiver2Thread.join();

        for (auto&& message : testMessages)
        {
            EXPECT_EQ(message.expectedData, message.receivedData);
        }
    }

    struct TestMessage
    {
        std::string expectedData;
        std::string receivedData;
    };

protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<TestMessage> testMessages;

    std::promise<void> allReceivedPromise;
    Callbacks callbacks;
};

TEST_F(ThreeCanControllerITest, test_can_ack_callbacks_fastrtps)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    ExecuteTest();
}

TEST_F(ThreeCanControllerITest, test_can_ack_callbacks_vasio)
{
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    ExecuteTest();
}

} // anonymous namespace
