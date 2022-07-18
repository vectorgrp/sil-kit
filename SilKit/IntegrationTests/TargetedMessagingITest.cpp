// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
#include "silkit/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {
using namespace std::chrono_literals;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;

TEST(TargetedMessagingITest, targeted_messaging)
{
    auto registryUri = MakeTestRegistryUri();

    std::vector<std::string> syncParticipantNames{ "Sender", "TargetReceiver" , "OtherReceiver" };

    auto receiveCount = 0;

    SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, registryUri);

    auto* senderComSimPart = testHarness.GetParticipant("Sender");
    auto* senderCom = dynamic_cast<SilKit::Core::IParticipantInternal*>(senderComSimPart->Participant());

    auto systemCtrl = senderComSimPart->GetOrCreateSystemController();

    auto* senderLifecycleService = senderComSimPart->GetOrCreateLifecycleServiceWithTimeSync();
    auto* senderTimeSyncService = senderLifecycleService->GetTimeSyncService();

    auto* senderCan =
        dynamic_cast<SilKit::Services::Can::CanController*>(senderCom->CreateCanController("CAN1", "CAN1"));
    senderCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << ": 'Sender' received targeted message from controller '" << controller << "'";
    });

    senderTimeSyncService->SetSimulationStepHandler(
        [&systemCtrl, &senderCan, &senderCom](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            std::cout << "Sender: Current time=" << nowMs.count() << "ms" << std::endl;
            if (now == 0ms)
            {
                SilKit::Services::Can::WireCanFrameEvent msg{};
                msg.direction = SilKit::Services::TransmitDirection::RX;
                msg.frame.canId = 42;
                senderCom->SendMsg(senderCan, "TargetReceiver", msg);
            }

            if (now == 3ms) { systemCtrl->Stop(); }
        }, 1ms);

    
    auto* receiverComSimPart = testHarness.GetParticipant("TargetReceiver");
    auto* receiverCom = dynamic_cast<SilKit::Core::IParticipantInternal*>(receiverComSimPart->Participant());
    auto* receiverLifecycleService = receiverComSimPart->GetOrCreateLifecycleServiceWithTimeSync();
    auto* receiverTimeSyncService = receiverLifecycleService->GetTimeSyncService();

    receiverTimeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
    }, 1ms);

    auto* receiverCan = receiverCom->CreateCanController("CAN1", "CAN1");

    receiverCan->AddFrameHandler(
        [&receiveCount](SilKit::Services::Can::ICanController* controller, auto msg) {
            std::cout << "'TargetReceiver' received a message from controller '" << controller
                      << "' with canId=" << msg.frame.canId << std::endl;
            ASSERT_TRUE(msg.frame.canId == 42) << "The received canId is wrong. expected=42; received=" << msg.frame.canId;
            receiveCount++;
        });

    auto* otherReceiverComSimPart = testHarness.GetParticipant("OtherReceiver");
    auto* otherReceiverCom = dynamic_cast<SilKit::Core::IParticipantInternal*>(otherReceiverComSimPart->Participant());
    auto* otherLifecycleService = otherReceiverComSimPart->GetOrCreateLifecycleServiceWithTimeSync();
    auto* otherTimeSyncService = otherLifecycleService->GetTimeSyncService();
    
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
