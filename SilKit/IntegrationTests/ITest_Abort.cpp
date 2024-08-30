/* Copyright (c) 2023 Vector Informatik GmbH

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

#include "ITest_MultiThreadedParticipants.hpp"

#include <list>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/orchestration/string_utils.hpp"

namespace {

using namespace testing;

class ITest_Abort : public ITest_MultiThreadedParticipants
{
};

TEST_F(ITest_Abort, test_Abort_Paused_Simulation_Sync)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    // Ensure that we do not end up in an error state after abort is called.
    // Thus, I need to ensure that the sync participants do not end up in an error
    // state.
    int size = static_cast<int>(syncParticipants.size() + 1); // include system controller
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Paused)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Paused)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Aborting)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(0);

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

    // Each participant knows that all other participants have started.
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
    AbortSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    // Expect shutdown state, which should happen after the abort handlers are called.
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

TEST_F(ITest_Abort, test_Abort_Running_Simulation_Sync)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    // Ensure that we do not end up in an error state after abort is called.
    // Thus, I need to ensure that the sync participants do not end up in an error
    // state.
    int size = static_cast<int>(syncParticipants.size() + 1); // include system controller
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Running)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Aborting)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(0);

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    RunParticipants(monitorParticipants);
    for (auto& p : monitorParticipants)
    {
        p.AwaitRunning();
    }

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    RunParticipants(syncParticipants);

    // Each participant knows that all other participants have started.
    for (auto& p : syncParticipants)
    {
        p.AwaitRunning();
    }
    systemControllerParticipant.AwaitRunning();
    AbortSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    // Expect shutdown state, which should happen after the abort handlers are called.
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

TEST_F(ITest_Abort, test_Abort_Stopped_Simulation_Sync)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    // Ensure that we do not end up in an error state after abort is called.
    // Thus, I need to ensure that the sync participants do not end up in an error
    // state.
    int size = static_cast<int>(syncParticipants.size() + 1); // include system controller
    int requiredSize = static_cast<int>(syncParticipants.size());
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Stopping)).Times(Between(0, size));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Stopped)).Times(Between(0, size));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Running)).Times(Between(0, requiredSize));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(size);
    // The system controller stops the simulation and aborts in the stop handler.
    // However it can happen that a participant is done with the stop before the abort arrives.
    // Thus we can only expect 1 to N aborts.
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Aborting)).Times(Between(1, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(Between(1, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(Between(0, size));
    // A participant may stop and shut down before the abort arrives. In this case ShuttingDown is reached.
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(Between(0, size));

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    // Start Monitor
    RunParticipants(monitorParticipants);
    for (auto& p : monitorParticipants)
    {
        p.AwaitRunning();
    }

    // Start System Controller
    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    auto setup = [this]() {
        systemControllerParticipant.lifecycleService->SetStopHandler(
            [this]() { this->systemControllerParticipant.systemController->AbortSimulation(); });
    };

    // Each participant knows that all other participants have started.
    RunParticipants(syncParticipants);
    RunSystemController(required, setup);

    for (auto& p : syncParticipants)
    {
        p.AwaitRunning();
    }
    systemControllerParticipant.AwaitRunning();

    StopSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    // Expect shutdown state, which should happen after the abort handlers are called.
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

TEST_F(ITest_Abort, test_Abort_Communication_Ready_Simulation_Sync)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    // Ensure that we do not end up in an error state after abort is called.
    // Thus, I need to ensure that the sync participants do not end up in an error
    // state.
    int size = static_cast<int>(syncParticipants.size() + 1); // include system controller
    int requiredParticipantsSize = static_cast<int>(syncParticipants.size());
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::ServicesCreated)).Times(Between(0, size));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::CommunicationInitializing))
        .Times(Between(0, requiredParticipantsSize));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::CommunicationInitialized)).Times(Between(0, size));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::ReadyToRun)).Times(Between(0, size));
    EXPECT_CALL(callbacks, AbortHandler(ParticipantState::Running)).Times(Between(0, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Shutdown)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Error)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Invalid)).Times(0);

    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ServicesCreated)).Times(Between(1, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitializing))
        .Times(Between(1, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::CommunicationInitialized)).Times(Between(1, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ReadyToRun)).Times(Between(0, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Running)).Times(Between(0, size));
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Aborting)).Times(size);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopping)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::Stopped)).Times(0);
    EXPECT_CALL(callbacks, ParticipantStateHandler(ParticipantState::ShuttingDown)).Times(0);

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});
    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    // Start System Monitor
    RunParticipants(monitorParticipants);
    for (auto& p : monitorParticipants)
    {
        p.AwaitRunning();
    }

    // Start System Controller
    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    auto setup = [this]() {
        systemControllerParticipant.lifecycleService->SetCommunicationReadyHandler(
            [this]() { this->systemControllerParticipant.systemController->AbortSimulation(); });
    };
    RunParticipants(syncParticipants);
    RunSystemController(required, setup);

    systemControllerParticipant.AwaitCommunicationInitialized();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    monitorParticipants.front().i.stopRequested = true;
    JoinParticipantThreads(_participantThreads_Async_Autonomous);

    // Expect shutdown state, which should happen after the abort handlers are called.
    for (const auto& participant : monitorParticipants.front().CopyMonitoredParticipantStates())
    {
        if (participant.first != "MonitorParticipant1")
        {
            EXPECT_EQ(participant.second, ParticipantState::Shutdown);
        }
    }

    StopRegistry();
}

} // anonymous namespace
