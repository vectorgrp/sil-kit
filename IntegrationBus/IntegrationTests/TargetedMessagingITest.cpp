// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "SimTestHarness.hpp"
#include "IParticipantInternal.hpp"
#include "ib/sim/can/all.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "CanControllerFacade.hpp"

#include "ib/mw/sync/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {
using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;

TEST(TargetedMessagingITest, targeted_messaging)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::vector<std::string> syncParticipantNames{ "Sender", "TargetReceiver" , "OtherReceiver" };

    auto receiveCount = 0;

    ib::test::SimTestHarness testHarness(syncParticipantNames, domainId);

    auto* senderCom = dynamic_cast<ib::mw::IParticipantInternal*>(testHarness.GetParticipant("Sender")->Participant());

    auto systemCtrl = senderCom->GetSystemController();

    auto* senderParticipant = senderCom->GetParticipantController();
    auto* senderCan = dynamic_cast<ib::sim::can::CanControllerFacade*>(senderCom->CreateCanController("CAN1"));
    senderCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << ": 'Sender' received targeted message from controller '" << controller << "'";
    });
    senderParticipant->SetPeriod(1ms);
    senderParticipant->SetSimulationTask(
        [&systemCtrl, &senderCan, &senderCom](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            std::cout << "Sender: Current time=" << nowMs.count() << "ms" << std::endl;
            if (now == 0ms)
            {
                ib::sim::can::CanFrameEvent msg;
                msg.frame.canId = 42;
                senderCom->SendIbMessage(senderCan, "TargetReceiver", msg);
            }

            if (now == 3ms) { systemCtrl->Stop(); }
        });

    auto* receiverCom =
        dynamic_cast<ib::mw::IParticipantInternal*>(testHarness.GetParticipant("TargetReceiver")->Participant());
    auto* receiverParticipant = receiverCom->GetParticipantController();
    receiverParticipant->SetPeriod(1ms);
    receiverParticipant->SetSimulationTask([](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
    });

    auto* receiverCan = receiverCom->CreateCanController("CAN1");

    receiverCan->AddFrameHandler(
        [&receiveCount](ib::sim::can::ICanController* controller, auto msg) {
            std::cout << "'TargetReceiver' received a message from controller '" << controller
                      << "' with canId=" << msg.frame.canId << std::endl;
            ASSERT_TRUE(msg.frame.canId == 42) << "The received canId is wrong. expected=42; received=" << msg.frame.canId;
            receiveCount++;
        });

    auto* otherReceiverCom =
        dynamic_cast<ib::mw::IParticipantInternal*>(testHarness.GetParticipant("OtherReceiver")->Participant());
    auto* otherReceiverParticipant = otherReceiverCom->GetParticipantController();
    otherReceiverParticipant->SetPeriod(1ms);
    otherReceiverParticipant->SetSimulationTask(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {});

    auto* otherReceiverCan = otherReceiverCom->CreateCanController("CAN1");

    otherReceiverCan->AddFrameHandler([](auto controller, auto) {
        FAIL() << "'otherReceiver' received targeted message from controller '" << controller << "'";
    });

    ASSERT_TRUE(testHarness.Run(3s));

    ASSERT_GT(receiveCount, 0) << "ReceiveCount was 0 - receiver likely never received the message.";
}

} // anonymous namespace
