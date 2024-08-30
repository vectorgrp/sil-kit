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

#include <iostream>
#include <unordered_map>
#include <cstdlib>

#include "silkit/services/all.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Can;

using testing::_;
using testing::A;
using testing::An;
using testing::AnyNumber;
using testing::AtLeast;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

auto MakeUserContext(uint16_t controllerId, uint16_t frameCounter) -> void*
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(controllerId) << 16 | static_cast<uintptr_t>(frameCounter));
}

bool CheckUserContextContainsControllerId(void* userContext, uint16_t controllerId)
{
    return ((reinterpret_cast<uintptr_t>(userContext) >> 16) & 0xFFFF) == controllerId;
}

bool CheckUserContextContainsFrameCounter(void* userContext, uint16_t frameCounter)
{
    return (reinterpret_cast<uintptr_t>(userContext) & 0xFFFF) == frameCounter;
}

MATCHER_P(UserContextContainsFrameCounter, frameCounter, "")
{
    if (CheckUserContextContainsFrameCounter(arg, frameCounter))
    {
        return true;
    }

    *result_listener << "user context pointer " << arg << " does not contains the frame counter " << frameCounter;
    return false;
}

auto AnAckWithCanIdAndFrameCounter(uint32_t canId,
                                   uint16_t frameCounter) -> testing::Matcher<const CanFrameTransmitEvent&>
{
    using namespace testing;
    return AllOf(Field(&CanFrameTransmitEvent::canId, canId),
                 Field(&CanFrameTransmitEvent::userContext, UserContextContainsFrameCounter(frameCounter)));
}

class ITest_ThreeCanController : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(AckHandler, void(const CanFrameTransmitEvent&));
    };

protected:
    ITest_ThreeCanController()
    {
        testMessages.resize(5);
        for (auto index = 0u; index < testMessages.size(); index++)
        {
            std::stringstream messageBuilder;
            messageBuilder << "Test Message " << index;
            testMessages[index].expectedData = messageBuilder.str();
        }

        syncParticipantNames = {"CanWriter", "CanReader1", "CanReader2"};
    }

    void SetupWriter(SilKit::Tests::SimParticipant* writer)
    {
        auto* controller = writer->Participant()->CreateCanController("CAN1", "CAN1");
        controllerToId.emplace(controller, static_cast<uint16_t>(controllerToId.size()));
        ASSERT_LT(controllerToId[controller], std::numeric_limits<uint16_t>::max());

        controller->AddFrameTransmitHandler([this](ICanController* ctrl, const CanFrameTransmitEvent& ack) {
            callbacks.AckHandler(ack);
            EXPECT_TRUE(CheckUserContextContainsControllerId(ack.userContext, controllerToId[ctrl]));
        });

        auto* lifecycleService = writer->GetOrCreateLifecycleService();

        lifecycleService->SetCommunicationReadyHandler([controller]() { controller->Start(); });

        auto name = writer->Name();
        lifecycleService->SetStopHandler(
            [name]() { std::cout << "Stop received by participant " << name << std::endl; });
        lifecycleService->SetShutdownHandler(
            [name]() { std::cout << "Shutdown received by participant " << name << std::endl; });

        auto* timeSyncService = writer->GetOrCreateTimeSyncService();
        timeSyncService->SetSimulationStepHandler([this, controller](auto, auto) {
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

                const auto frameCounter = numSent + 1;
                ASSERT_LT(numSent + 1, std::numeric_limits<uint16_t>::max());

                controller->SendFrame(msg,
                                      MakeUserContext(controllerToId[controller], static_cast<uint16_t>(frameCounter)));
                numSent++;
            }
        }, 1ms);
    }

    void SetupReader(SilKit::Tests::SimParticipant* reader)
    {
        auto* controller = reader->Participant()->CreateCanController("CAN1", "CAN1");
        controller->AddFrameTransmitHandler(
            [this](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) { callbacks.AckHandler(ack); });

        controller->AddFrameHandler([this, reader](ICanController*, const CanFrameEvent& msg) {
            if (reader->Name() == "CanReader1")
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

        auto* lifecycleService = reader->GetOrCreateLifecycleService();

        lifecycleService->SetCommunicationReadyHandler([controller]() { controller->Start(); });

        auto name = reader->Name();
        lifecycleService->SetStopHandler(
            [name]() { std::cout << "Stop received by participant " << name << std::endl; });
        lifecycleService->SetShutdownHandler(
            [name]() { std::cout << "Shutdown received by participant " << name << std::endl; });
    }

    void ExecuteTest()
    {
        SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, "silkit://localhost:0");

        auto* canWriter = testHarness.GetParticipant("CanWriter");
        SetupWriter(canWriter);

        auto* canReader1 = testHarness.GetParticipant("CanReader1");
        SetupReader(canReader1);

        auto* canReader2 = testHarness.GetParticipant("CanReader2");
        SetupReader(canReader2);

        for (uint16_t index = 1u; index <= static_cast<uint16_t>(testMessages.size()); index++)
        {
            EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndFrameCounter(1, index))).Times(1);
        }
        EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndFrameCounter(1, 0))).Times(0);
        EXPECT_CALL(callbacks, AckHandler(AnAckWithCanIdAndFrameCounter(1, 6))).Times(0);

        EXPECT_TRUE(testHarness.Run(30s))
            << "TestHarness timeout occurred!"
            << " numSent=" << numSent << " numReceived=" << numReceived << " numReceived2=" << numReceived2;

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

    std::unordered_map<ICanController*, uint16_t> controllerToId;

    unsigned numSent{0}, numReceived{0}, numReceived2{0};

    Callbacks callbacks;
};

TEST_F(ITest_ThreeCanController, test_can_ack_callbacks)
{
    ExecuteTest();
}

} // anonymous namespace
