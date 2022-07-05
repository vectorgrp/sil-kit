// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

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

class SingleParticipantITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const CanFrameTransmitEvent&));
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
        syncParticipantNames = {"CanWriter"};
    }

    void SetupWriter(ib::test::SimParticipant* participant)
    {

        auto* canController = participant->Participant()->CreateCanController("CAN1");
        canController->AddFrameTransmitHandler(
            [this, participant](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
                callbacks.AckHandler(ack);
                numAcked++;
                if (numAcked >= testMessages.size())
                {
                    participant->Stop();
                }
            });

        auto* lifecycleService = participant->Participant()->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        lifecycleService->SetCommunicationReadyHandler([canController]() {
            canController->Start();
        });

        timeSyncService->SetSimulationTask([this, canController, lifecycleService](auto, auto) {
            EXPECT_EQ(lifecycleService->State(), sync::ParticipantState::Running);
            if (numSent < testMessages.size())
            {
                const auto& message = testMessages.at(numSent);
                CanFrame msg;
                msg.canId = 1;
                msg.dataField.assign(message.expectedData.begin(), message.expectedData.end());
                msg.dlc = msg.dataField.size();

                canController->SendFrame(std::move(msg));
                numSent++;
                std::this_thread::sleep_for(100ms);
            }
        });
    }

    void ExecuteTest()
    {
        auto registryUri = MakeTestRegistryUri();
        ib::test::SimTestHarness testHarness(syncParticipantNames, registryUri);

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
    std::vector<std::string> syncParticipantNames;
    std::vector<TestMessage> testMessages;

    unsigned numSent{0},
        numAcked{0};

    Callbacks callbacks;
};

TEST_F(SingleParticipantITest, test_single_participant_vasio)
{
    ExecuteTest();
}

} // anonymous namespace
