// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "silkit/services/all.hpp"
#include "silkit/util/functional.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Can;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

auto AnAckWithCanIdAndUserContext(uint32_t canId, void* userContext) -> testing::Matcher<const CanFrameTransmitEvent&>
{
    using namespace testing;
    return AllOf(
        Field(&CanFrameTransmitEvent::canId, canId),
        Field(&CanFrameTransmitEvent::userContext, userContext)
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

    void SetupWriter(SilKit::Tests::SimParticipant* participant)
    {

        auto* canController = participant->Participant()->CreateCanController("CAN1", "CAN1");
        canController->AddFrameTransmitHandler(
            [this, participant](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
                callbacks.AckHandler(ack);
                numAcked++;
                if (numAcked >= testMessages.size())
                {
                    participant->Stop();
                }
            });

        auto* lifecycleService = participant->GetOrCreateLifecycleServiceWithTimeSync();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        lifecycleService->SetCommunicationReadyHandler([canController]() {
            canController->Start();
        });

        timeSyncService->SetSimulationStepHandler([this, canController, lifecycleService](auto, auto) {
            EXPECT_EQ(lifecycleService->State(), Services::Orchestration::ParticipantState::Running);
            if (numSent < testMessages.size())
            {
                const auto& message = testMessages.at(numSent);

                std::vector<uint8_t> expectedData;
                expectedData.resize(message.expectedData.size());
                std::copy(message.expectedData.begin(), message.expectedData.end(), expectedData.begin());

                CanFrame msg;
                msg.canId = 1;
                msg.dataField = expectedData;
                msg.dlc = static_cast<uint16_t>(msg.dataField.size());

                canController->SendFrame(std::move(msg), (void*)(static_cast<uintptr_t>(numSent + 1)));
                numSent++;
                std::this_thread::sleep_for(100ms);
            }
        }, 1ms);
    }

    void ExecuteTest()
    {
        auto registryUri = MakeTestRegistryUri();
        SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, registryUri);

        auto* canWriter = testHarness.GetParticipant("CanWriter");
        SetupWriter(canWriter);

        for (uintptr_t index = 1u; index <= testMessages.size(); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndUserContext(1, (void*)index))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndUserContext(1, (void*)uintptr_t(0)))).Times(0);
        EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndUserContext(1, (void*)uintptr_t(6)))).Times(0);

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
