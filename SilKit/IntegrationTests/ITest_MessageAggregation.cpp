// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <chrono>
#include <iostream>

#include "ITestFixture.hpp"

#include "gtest/gtest.h"

namespace {

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::PubSub;

using namespace std::chrono_literals;

struct ITest_MessageAggregation : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

TEST_F(ITest_MessageAggregation, receive_msg_after_lifecycle_has_been_stopped)
{
    std::promise<void> recvMsgPromise;
    auto recvMsgFuture = recvMsgPromise.get_future();

    SetupFromParticipantList({"Publisher", "Subscriber"});
    SilKit::Services::PubSub::PubSubSpec dataSpec{"someTopic", {}};

    {
        std::string participantName = "Publisher";
        std::string participantConfig(
            R"({"Experimental": {"TimeSynchronization": {"EnableMessageAggregation": "Auto"}}})");
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName, participantConfig);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& dataPublisher = participant->CreateDataPublisher("PubCtrl", dataSpec);

        timeSyncService->SetSimulationStepHandler(
            [dataPublisher, lifecycleService](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
            uint32_t messageSizeInBytes = 1;
            std::vector<uint8_t> data(messageSizeInBytes, '*');
            dataPublisher->Publish(std::move(data));

            // stop lifecycle immediately (one message is sent and should be received by subscriber)
            lifecycleService->Stop("Stop and check if message of current time step is transmitted.");
        }, 1ms);
    }

    {
        std::string participantName = "Subscriber";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        /*auto&& lifecycleService =*/simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        /*auto&& dataSubscriber =*/participant->CreateDataSubscriber(
            "PubCtrl", dataSpec,
            [&recvMsgPromise](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
            recvMsgPromise.set_value();
        });

        timeSyncService->SetSimulationStepHandler(
            [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {}, 1ms);
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    bool msgReceived = recvMsgFuture.wait_for(5s) == std::future_status::ready;
    EXPECT_TRUE(msgReceived)
        << "Message of current time step has not been received (flush of aggregated messages has not been performed).";
}

TEST_F(ITest_MessageAggregation, timeout_in_case_of_deadlock_when_using_async_sim_step_handler)
{
    SetupFromParticipantList({"Publisher", "Subscriber"});
    SilKit::Services::PubSub::PubSubSpec dataSpecPing{"ping", {}};
    SilKit::Services::PubSub::PubSubSpec dataSpecPong{"pong", {}};

    bool msgReceived{false};

    // participant with async simulation step handler & enabled message aggregation
    {
        std::string participantName = "Publisher";
        std::string participantConfig(
            R"({"Experimental": {"TimeSynchronization": {"EnableMessageAggregation": "On"}}})");
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName, participantConfig);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& dataPublisher = participant->CreateDataPublisher("Ctrl", dataSpecPing);
        /*auto&& dataSubscriber =*/participant->CreateDataSubscriber(
            "Ctrl", dataSpecPong,
            [timeSyncService, &msgReceived](IDataSubscriber* /*subscriber*/,
                                            const DataMessageEvent& /*dataMessageEvent*/) {
            // unblock, if message from other participant is received
            msgReceived = true;
            timeSyncService->CompleteSimulationStep();
        });

        timeSyncService->SetSimulationStepHandlerAsync(
            [dataPublisher, lifecycleService, timeSyncService, &msgReceived](std::chrono::nanoseconds /*now*/,
                                                                             std::chrono::nanoseconds /*duration*/) {
            // send ping
            std::vector<uint8_t> ping(1, '?');
            dataPublisher->Publish(std::move(ping));

            if (msgReceived)
            {
                lifecycleService->Stop("One time step has been performed successfully.");
            }
        },
            1s);
    }

    {
        std::string participantName = "Subscriber";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        /*auto&& lifecycleService =*/simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& dataPublisher = participant->CreateDataPublisher("Ctrl", dataSpecPong);
        /*auto&& dataSubscriber =*/participant->CreateDataSubscriber(
            "Ctrl", dataSpecPing,
            [dataPublisher](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
            // send back pong
            std::vector<uint8_t> pong(1, '!');
            dataPublisher->Publish(std::move(pong));
        });

        timeSyncService->SetSimulationStepHandlerAsync(
            [timeSyncService](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
            timeSyncService->CompleteSimulationStep();
        }, 1s);
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
}

} // namespace