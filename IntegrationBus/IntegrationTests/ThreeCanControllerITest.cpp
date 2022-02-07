// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <cstdlib>

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
        builder.SimulationSetup()
            .AddParticipant("CanReader1")
            .AddCan("CAN1").WithLink("CAN1");
        builder.SimulationSetup()
            .AddParticipant("CanReader2")
            .AddCan("CAN1").WithLink("CAN1");

        ibConfig = builder.Build();
    }

    void SetupWriter(ib::test::SimParticipant* writer)
    {

        auto* controller = writer->ComAdapter()->CreateCanController("CAN1", "CAN1");
        controller->RegisterTransmitStatusHandler(
            [this, writer](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);
        });

        auto* participantController = writer->ComAdapter()->GetParticipantController();
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

    void SetupReader(ib::test::SimParticipant* reader)
    {

        auto* controller = reader->ComAdapter()->CreateCanController("CAN1", "CAN1");
        controller->RegisterTransmitStatusHandler(
            [this](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
            callbacks.AckHandler(ack);
        });

        controller->RegisterReceiveMessageHandler(
            [this, reader](ICanController*, const CanMessage& msg) {

            if ( reader->Name() == "CanReader1")
            {
                std::string message(msg.dataField.begin(), msg.dataField.end());
                testMessages.at(numReceived++).receivedData = message;
                if (numReceived >= testMessages.size())
                {
                    reader->Stop();
                }
            }
            else
            {
                numReceived2++;
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

        auto* canReader1 = testHarness.GetParticipant("CanReader1");
        SetupReader(canReader1);

        auto* canReader2 = testHarness.GetParticipant("CanReader2");
        SetupReader(canReader2);


        for (auto index = 1u; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(index, 1))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(0, 1))).Times(0);
        EXPECT_CALL(callbacks, AckHandler(AnAckWithTxIdAndCanId(6, 1))).Times(0);


        EXPECT_TRUE(testHarness.Run(30s)) 
            << "TestHarness timeout occurred!"
            << " numSent=" << numSent
            << " numReceived=" << numReceived
            << " numReceived2=" << numReceived2
            ;

        for (auto&& message : testMessages)
        {
            EXPECT_EQ(message.expectedData, message.receivedData);
        }
        EXPECT_EQ(numSent, numReceived);
        EXPECT_EQ(numSent, numReceived2);
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
        numReceived{0},
        numReceived2{0};

    Callbacks callbacks;
};

TEST_F(ThreeCanControllerITest, test_can_ack_callbacks_vasio)
{
    ExecuteTest(ib::cfg::Middleware::VAsio);
}

} // anonymous namespace
