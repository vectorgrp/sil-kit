// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITest_MultiThreadedParticipants.hpp"

#include <list>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/orchestration/string_utils.hpp"

namespace {

using namespace testing;

class ITest_Lifecycle : public ITest_MultiThreadedParticipants
{
};

TEST_F(ITest_Lifecycle, test_Stop_Paused_Simulation_Sync)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    // Stopping a paused simulation must run through the regular stop sequence, without errors or aborts.
    int size = static_cast<int>(syncParticipants.size() + 1); // include system controller
    EXPECT_CALL(callbacks, AbortHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Aborting)).Times(0);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Paused)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(size);

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunParticipants(monitorParticipants);
    for (auto& p : monitorParticipants)
    {
        p.AwaitRunning();
    }

    RunSystemController(required);
    RunParticipants(syncParticipants);

    for (auto& p : syncParticipants)
    {
        p.AwaitRunning();
    }
    systemControllerParticipant.AwaitRunning();
    PauseSystemController();
    for (auto& p : syncParticipants)
    {
        p.lifecycleService->Pause("Pause Test.");
    }
    for (auto& p : syncParticipants)
    {
        p.AwaitPaused();
    }

    // The resulting SystemState::Stopping makes the paused sync participants stop as well
    StopSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    for (const auto& participant : monitorParticipants.front().CopyMonitoredParticipantStates())
    {
        if (participant.first != "MonitorParticipant1")
        {
            EXPECT_EQ(participant.second, ParticipantState::Shutdown);
        }
    }

    monitorParticipants.front().i.stopRequested = true;
    JoinParticipantThreads(_participantThreads_Async_Autonomous);

    StopRegistry();
}

} // anonymous namespace
