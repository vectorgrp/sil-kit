// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <cstdlib>

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

auto AnAckWithTxIdAndCanId(CanTxId transmitId, uint32_t canId) -> testing::Matcher<const CanFrameTransmitEvent&>
{
    using namespace testing;
    return AllOf(
        Field(&CanFrameTransmitEvent::transmitId, transmitId),
        Field(&CanFrameTransmitEvent::canId, canId)
    );
}

class ThreeCanControllerITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const CanFrameTransmitEvent&));
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

        syncParticipantNames = { "CanWriter", "CanReader1", "CanReader2" };
    }

    void SetupWriter(ib::test::SimParticipant* writer)
    {

        auto* controller = writer->Participant()->CreateCanController("CAN1", "CAN1");
        controller->AddFrameTransmitHandler(
            [this](ICanController* ctrl, const CanFrameTransmitEvent& ack) {
            callbacks.AckHandler(ack);
            EXPECT_EQ(reinterpret_cast<void*>(ctrl), ack.userContext);
        });

        auto* lifecycleService = writer->Participant()->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();
        timeSyncService->SetSimulationTask(
            [this, controller](auto, auto)
            {
                if (numSent < testMessages.size())
                {
                    const auto& message = testMessages.at(numSent);
                    CanFrame msg;
                    msg.canId = 1;
                    msg.dataField.assign(message.expectedData.begin(), message.expectedData.end());
                    msg.dlc = msg.dataField.size();

                    controller->SendFrame(std::move(msg), reinterpret_cast<void*>(controller));
                    numSent++;
                    std::this_thread::sleep_for(100ms);
                }
        });
    }

    void SetupReader(ib::test::SimParticipant* reader)
    {

        auto* controller = reader->Participant()->CreateCanController("CAN1", "CAN1");
        controller->AddFrameTransmitHandler(
            [this](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
            callbacks.AckHandler(ack);
        });

        controller->AddFrameHandler(
            [this, reader](ICanController*, const CanFrameEvent& msg) {

            if ( reader->Name() == "CanReader1")
            {
                std::string message(msg.frame.dataField.begin(), msg.frame.dataField.end());
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

    void ExecuteTest()
    {
        auto registryUri = MakeTestRegistryUri();
        ib::test::SimTestHarness testHarness(syncParticipantNames, registryUri);

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
    std::vector<std::string> syncParticipantNames;

    std::vector<TestMessage> testMessages;

    unsigned numSent{0},
        numReceived{0},
        numReceived2{0};

    Callbacks callbacks;
};

TEST_F(ThreeCanControllerITest, test_can_ack_callbacks)
{
    ExecuteTest();
}

} // anonymous namespace
