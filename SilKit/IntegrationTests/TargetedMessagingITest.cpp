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

#include "silkit/core/sync/all.hpp"
#include "silkit/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {
using namespace std::chrono_literals;
using namespace SilKit::Core;
using namespace SilKit::Core::Orchestration;

TEST(TargetedMessagingITest, targeted_messaging)
{
    auto registryUri = MakeTestRegistryUri();

    std::vector<std::string> syncParticipantNames{ "Sender", "TargetReceiver" , "OtherReceiver" };

    auto receiveCount = 0;

    SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, registryUri);

    auto* senderCom = dynamic_cast<SilKit::Core::IParticipantInternal*>(testHarness.GetParticipant("Sender")->Participant());

    auto systemCtrl = senderCom->GetSystemController();

    auto* senderLifecycleService = senderCom->GetLifecycleService();
    auto* senderTimeSyncService = senderLifecycleService->GetTimeSyncService();

    auto* senderCan = dynamic_cast<SilKit::Services::Can::CanController*>(senderCom->CreateCanController("CAN1"));
    senderCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << ": 'Sender' received targeted message from controller '" << controller << "'";
    });
    senderTimeSyncService->SetPeriod(1ms);
    senderTimeSyncService->SetSimulationTask(
        [&systemCtrl, &senderCan, &senderCom](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            std::cout << "Sender: Current time=" << nowMs.count() << "ms" << std::endl;
            if (now == 0ms)
            {
                SilKit::Services::Can::CanFrameEvent msg{};
                msg.direction = SilKit::Services::TransmitDirection::RX;
                msg.frame.canId = 42;
                senderCom->SendMsg(senderCan, "TargetReceiver", msg);
            }

            if (now == 3ms) { systemCtrl->Stop(); }
        });

    auto* receiverCom =
        dynamic_cast<SilKit::Core::IParticipantInternal*>(testHarness.GetParticipant("TargetReceiver")->Participant());
    auto* receiverLifecycleService = receiverCom->GetLifecycleService();
    auto* receiverTimeSyncService = receiverLifecycleService->GetTimeSyncService();

    receiverTimeSyncService->SetPeriod(1ms);
    receiverTimeSyncService->SetSimulationTask(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
    });

    auto* receiverCan = receiverCom->CreateCanController("CAN1");

    receiverCan->AddFrameHandler(
        [&receiveCount](SilKit::Services::Can::ICanController* controller, auto msg) {
            std::cout << "'TargetReceiver' received a message from controller '" << controller
                      << "' with canId=" << msg.frame.canId << std::endl;
            ASSERT_TRUE(msg.frame.canId == 42) << "The received canId is wrong. expected=42; received=" << msg.frame.canId;
            receiveCount++;
        });

    auto* otherReceiverCom =
        dynamic_cast<SilKit::Core::IParticipantInternal*>(testHarness.GetParticipant("OtherReceiver")->Participant());
    auto* otherLifecycleService = otherReceiverCom->GetLifecycleService();
    auto* otherTimeSyncService = otherLifecycleService->GetTimeSyncService();
    otherTimeSyncService->SetPeriod(1ms);
    otherTimeSyncService->SetSimulationTask(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {});

    auto* otherReceiverCan = otherReceiverCom->CreateCanController("CAN1");

    otherReceiverCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << "'otherReceiver' received targeted message from controller '" << controller << "'";
    });

    ASSERT_TRUE(testHarness.Run(3s));

    ASSERT_GT(receiveCount, 0) << "ReceiveCount was 0 - receiver likely never received the message.";
}

} // anonymous namespace
