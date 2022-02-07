// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


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

class SingleParticipantITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const CanTransmitAcknowledge&));
    };

protected:
    SingleParticipantITest()
    {
        testMessages.resize(5);
        for (auto index = 0u; index < testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            testMessages[index].expectedData = messageBuilder.str();
        }

        ib::cfg::ConfigBuilder builder{"TestConfig"};

        builder.SimulationSetup()
            .AddParticipant("CanWriter")
            .AddCan("CAN1").WithLink("CAN1");

        ibConfig = builder.Build();
    }

    void SetupWriter(ib::test::SimParticipant* participant)
    {

        auto* controller = participant->ComAdapter()->CreateCanController("CAN1", "CAN1");
        controller->RegisterTransmitStatusHandler(
            [this, participant](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
                callbacks.AckHandler(ack);
                numAcked++;
                if (numAcked >= testMessages.size())
                {
                    participant->Stop();
                }
            });

        auto* participantController = participant->ComAdapter()->GetParticipantController();
        participantController->SetSimulationTask(
            [this, controller](auto, auto)
            {
                if (numSent < testMessages.size())
                {
                    const auto& message = testMessages.at(numSent);
                    CanMessage msg;
                    msg.canId = 1;
                    msg.dataField.assign(message.expectedData.begin(), message.expectedData.end());
                    msg.dlc = msg.dataField.size();

                    controller->SendMessage(std::move(msg));
                    numSent++;
                    std::this_thread::sleep_for(100ms);
                }
        });
    }

    void ExecuteTest(ib::cfg::Middleware middleware)
    {
        ibConfig.middlewareConfig.activeMiddleware = middleware;
        const uint32_t domainId = static_cast<uint32_t>(GetTestPid());
        ib::test::SimTestHarness testHarness(ibConfig, domainId);

        auto* canWriter = testHarness.GetParticipant("CanWriter");
        SetupWriter(canWriter);

        for (auto index = 1u; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(index, 1))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(0, 1))).Times(0);
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(6, 1))).Times(0);

        EXPECT_TRUE(testHarness.Run(30s))
            << "TestHarness timeout occurred!"
            << " numSent=" << numSent
            << " numAcked=" << numAcked
            ;

        EXPECT_EQ(numSent, testMessages.size());
        EXPECT_EQ(numAcked, testMessages.size());
    }

    struct TestMessage
    {
        std::string expectedData;
        std::string receivedData;
    };

protected:
    ib::cfg::Config ibConfig;

    std::vector<TestMessage> testMessages;

    unsigned numSent{0},
        numAcked{0};

    Callbacks callbacks;
};

TEST_F(SingleParticipantITest, test_single_participant_vasio)
{
    ExecuteTest(ib::cfg::Middleware::VAsio);
}

} // anonymous namespace
