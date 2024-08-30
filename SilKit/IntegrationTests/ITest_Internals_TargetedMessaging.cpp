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

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "SimTestHarness.hpp"
#include "IParticipantInternal.hpp"
#include "silkit/services/can/all.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "CanController.hpp"

#include "silkit/services/orchestration/all.hpp"
#include "functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "InternalHelpers.hpp"

namespace {
using namespace std::chrono_literals;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;

TEST(ITest_Internals_TargetedMessaging, targeted_messaging)
{
    std::vector<std::string> syncParticipantNames{"Sender", "TargetReceiver", "OtherReceiver"};

    auto receiveCount = 0;

    SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, "silkit://localhost:0", false);

    auto* senderComSimPart = testHarness.GetParticipant("Sender");
    auto* senderCom = &SilKit::Tests::ToParticipantInternal(*senderComSimPart->Participant());

    auto* senderLifecycleService = senderComSimPart->GetOrCreateLifecycleService();
    auto* senderTimeSyncService = senderComSimPart->GetOrCreateTimeSyncService();

    auto* senderCan =
        dynamic_cast<SilKit::Services::Can::CanController*>(senderCom->CreateCanController("CAN1", "CAN1"));
    senderCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << ": 'Sender' received targeted message from controller '" << controller << "'";
    });

    senderTimeSyncService->SetSimulationStepHandler(
        [&senderLifecycleService, &senderCan, &senderCom](std::chrono::nanoseconds now,
                                                          std::chrono::nanoseconds /*duration*/) {
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
        std::cout << "Sender: Current time=" << nowMs.count() << "ms" << std::endl;
        if (now == 0ms)
        {
            SilKit::Services::Can::WireCanFrameEvent msg{};
            msg.direction = SilKit::Services::TransmitDirection::RX;
            msg.frame.canId = 42;
            senderCom->SendMsg(senderCan, "TargetReceiver", msg);
        }

        if (now == 3ms)
        {
            senderLifecycleService->Stop("Test");
        }
    },
        1ms);


    auto* receiverComSimPart = testHarness.GetParticipant("TargetReceiver");
    auto* receiverCom = &SilKit::Tests::ToParticipantInternal(*receiverComSimPart->Participant());
    auto* receiverTimeSyncService = receiverComSimPart->GetOrCreateTimeSyncService();

    receiverTimeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {}, 1ms);

    auto* receiverCan = receiverCom->CreateCanController("CAN1", "CAN1");

    receiverCan->AddFrameHandler([&receiveCount](SilKit::Services::Can::ICanController* controller, auto msg) {
        std::cout << "'TargetReceiver' received a message from controller '" << controller
                  << "' with canId=" << msg.frame.canId << std::endl;
        ASSERT_TRUE(msg.frame.canId == 42) << "The received canId is wrong. expected=42; received=" << msg.frame.canId;
        receiveCount++;
    });

    auto* otherReceiverComSimPart = testHarness.GetParticipant("OtherReceiver");
    auto* otherReceiverCom = &SilKit::Tests::ToParticipantInternal(*otherReceiverComSimPart->Participant());
    auto* otherTimeSyncService = otherReceiverComSimPart->GetOrCreateTimeSyncService();

    otherTimeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {}, 1ms);

    auto* otherReceiverCan = otherReceiverCom->CreateCanController("CAN1", "CAN1");

    otherReceiverCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << "'otherReceiver' received targeted message from controller '" << controller << "'";
    });

    ASSERT_TRUE(testHarness.Run(3s));

    ASSERT_GT(receiveCount, 0) << "ReceiveCount was 0 - receiver likely never received the message.";
}

} // anonymous namespace
