// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "ITestFixture.hpp"

#include "silkit/services/can/all.hpp"

using namespace std::chrono_literals;

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Services::Can;
using namespace SilKit::Services::Orchestration;

struct Reception
{
    std::chrono::nanoseconds now;
    std::chrono::nanoseconds timestamp;
    std::chrono::nanoseconds sendTime;
};

struct ITest_ExactMode : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;

    static auto MakeConfig(const std::string& networkLatencies) -> std::string
    {
        return R"({"Experimental":{"TimeSynchronization":{"Latency":10,"NetworkLatencies":[)" + networkLatencies
               + "]}}}";
    }

    static void StartOnCommunicationReady(ILifecycleService* lifecycleService, std::vector<ICanController*> controllers)
    {
        lifecycleService->SetCommunicationReadyHandler([controllers] {
            for (auto* controller : controllers)
            {
                controller->SetBaudRate(500'000, 0, 0);
                controller->Start();
            }
        });
    }
};

// Receivers delay frames by the latency they configure for the network, and deliver each frame in the step that
// starts at its due time.
TEST_F(ITest_ExactMode, frames_are_delivered_at_send_time_plus_the_receivers_network_latency)
{
    const auto stepSize = 1ms;
    const auto stopAt = 60ms;

    SetupFromParticipantList({"Sender", "ReceiverCan1", "ReceiverCan2"},
                             /*enableInternalSystemMonitorTimeSync=*/false);

    std::mutex mx;
    std::map<std::string, std::vector<Reception>> receptions;

    {
        auto* sender = _simTestHarness->GetParticipant("Sender", MakeConfig(""));
        auto* participant = sender->Participant();
        auto* can1 = participant->CreateCanController("Can1", "CAN1");
        auto* can2 = participant->CreateCanController("Can2", "CAN2");
        auto* lifecycleService = sender->GetOrCreateLifecycleService();
        StartOnCommunicationReady(lifecycleService, {can1, can2});

        sender->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
            [=](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
            if (now >= stopAt)
            {
                lifecycleService->Stop("done");
                return;
            }
            // The payload carries the send time in milliseconds
            std::vector<uint8_t> payload{static_cast<uint8_t>(now / 1ms)};
            CanFrame frame{};
            frame.canId = 1;
            frame.dlc = 1;
            frame.dataField = SilKit::Util::ToSpan(payload);
            can1->SendFrame(frame);
            can2->SendFrame(frame);
        }, stepSize);
    }

    for (const auto& receiver : {std::make_pair("ReceiverCan1", R"({"Network":"CAN1","Latency":2})"),
                                 std::make_pair("ReceiverCan2", R"({"Network":"CAN2","Latency":5})")})
    {
        const std::string name = receiver.first;
        auto* simParticipant = _simTestHarness->GetParticipant(name, MakeConfig(receiver.second));
        auto* participant = simParticipant->Participant();
        auto* can = participant->CreateCanController("Can", name == "ReceiverCan1" ? "CAN1" : "CAN2");
        auto* timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        StartOnCommunicationReady(simParticipant->GetOrCreateLifecycleService(), {can});

        can->AddFrameHandler([&mx, &receptions, name, timeSyncService](ICanController*, const CanFrameEvent& event) {
            std::lock_guard<std::mutex> lock{mx};
            receptions[name].push_back(
                Reception{timeSyncService->Now(), event.timestamp, event.frame.dataField[0] * 1ms});
        });
        timeSyncService->SetSimulationStepHandler([](auto, auto) {}, stepSize);
    }

    ASSERT_TRUE(_simTestHarness->Run(10s)) << "SimTestHarness should terminate without timeout";

    for (const auto& expected : {std::make_pair("ReceiverCan1", 2ms), std::make_pair("ReceiverCan2", 5ms)})
    {
        const auto& received = receptions[expected.first];
        const auto latency = expected.second;
        SCOPED_TRACE(expected.first);

        // The simulation stops when the sender reaches stopAt; peers may stop up to a latency earlier
        const auto lastCertainSendTime = stopAt - 10ms - 2 * stepSize;
        ASSERT_GE(received.size(), static_cast<size_t>(lastCertainSendTime / stepSize));

        for (size_t i = 0; i < received.size(); ++i)
        {
            const auto& reception = received[i];
            EXPECT_EQ(reception.sendTime, static_cast<std::chrono::nanoseconds::rep>(i) * stepSize)
                << "frames are delivered in order, none is lost";
            EXPECT_EQ(reception.timestamp, reception.sendTime + latency) << "at index " << i;
            EXPECT_EQ(reception.now, reception.timestamp) << "delivered in the step that starts at its due time";
        }
    }
}

} // anonymous namespace
