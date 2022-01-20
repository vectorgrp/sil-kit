// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "SimTestHarness.hpp"
#include "IComAdapterInternal.hpp"
#include "ib/sim/can/all.hpp"
#include "ib/sim/can/CanDatatypes.hpp"
#include "CanControllerFacade.hpp"

#include "ib/cfg/ConfigBuilder.hpp"
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

    // Create a minimal IbConfig
    ib::cfg::ConfigBuilder builder{"TestConfig"};

    builder.ConfigureVAsio();

    builder.SimulationSetup().AddParticipant("Sender").AddCan("CAN1");
    builder.SimulationSetup().AddParticipant("TargetReceiver").AddCan("CAN1");
    builder.SimulationSetup().AddParticipant("OtherReceiver").AddCan("CAN1");

    auto ibConfig = builder.Build();

    auto receiveCount = 0;

    ib::test::SimTestHarness testHarness(ibConfig, domainId);

    auto* senderCom = dynamic_cast<ib::mw::IComAdapterInternal*>(testHarness.GetParticipant("Sender")->ComAdapter());

    auto systemCtrl = senderCom->GetSystemController();

    auto* senderParticipant = senderCom->GetParticipantController();
    auto* senderCan = dynamic_cast<ib::sim::can::CanControllerFacade*>(senderCom->CreateCanController("CAN1"));
    senderCan->RegisterReceiveMessageHandler([](auto controller, auto) {
        FAIL() << ": 'Sender' received targeted message from controller '" << controller << "'";
    });
    senderParticipant->SetPeriod(1ms);
    senderParticipant->SetSimulationTask(
        [&systemCtrl, &senderCan, &senderCom](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            std::cout << "Sender: Current time=" << nowMs.count() << "ms" << std::endl;
            if (now == 0ms)
            {
                ib::sim::can::CanMessage msg;
                msg.canId = 42;
                senderCom->SendIbMessage(senderCan, "TargetReceiver", msg);
            }

            if (now == 3ms) { systemCtrl->Stop(); }
        });

    auto* receiverCom =
        dynamic_cast<ib::mw::IComAdapterInternal*>(testHarness.GetParticipant("TargetReceiver")->ComAdapter());
    auto* receiverParticipant = receiverCom->GetParticipantController();
    receiverParticipant->SetPeriod(1ms);
    receiverParticipant->SetSimulationTask(
        [&systemCtrl, &senderCan](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {});
    auto* receiverCan = receiverCom->CreateCanController("CAN1");
    receiverCan->RegisterReceiveMessageHandler(
        [&receiveCount, &receiverCan](ib::sim::can::ICanController* controller, auto msg) {
            std::cout << "'TargetReceiver' received a message from controller '" << controller
                      << "' with canId=" << msg.canId << std::endl;
            ASSERT_TRUE(receiverCan != controller) << "Received message from self.";
            ASSERT_TRUE(msg.canId == 42) << "The received canId is wrong. expected=42; received=" << msg.canId;
            receiveCount++;
        });

    auto* otherReceiverCom =
        dynamic_cast<ib::mw::IComAdapterInternal*>(testHarness.GetParticipant("OtherReceiver")->ComAdapter());
    auto* otherReceiverParticipant = otherReceiverCom->GetParticipantController();
    otherReceiverParticipant->SetPeriod(1ms);
    otherReceiverParticipant->SetSimulationTask(
        [&systemCtrl, &senderCan](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {});
    auto* otherReceiverCan = otherReceiverCom->CreateCanController("CAN1");
    otherReceiverCan->RegisterReceiveMessageHandler([](auto controller, auto) {
        FAIL() << "'otherReceiver' received targeted message from controller '" << controller << "'";
    });

    ASSERT_TRUE(testHarness.Run(3s));
    systemCtrl->Run();

    ASSERT_GT(receiveCount, 0) << "ReceiveCount was 0 - receiver likely never received the message.";
}

} // anonymous namespace
