// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "LifecycleStateMachine.hpp"

namespace {

using namespace SilKit::Services::Orchestration;

struct FakeEffects
{
    OperationMode operationMode{OperationMode::Coordinated};
    bool timeSyncActive{false};
    CallbackResult communicationReadyResult{CallbackResult::Completed};
    bool startingResult{true};
    bool stopResult{true};
    bool shutdownResult{true};
    bool abortResult{true};
    ParticipantState lastAbortState{ParticipantState::Invalid};
    std::vector<ParticipantState> publishedStates;

    struct Logger
    {
        void Warn(const std::string& msg)
        {
            warnings.push_back(msg);
        }

        void Info(const std::string& msg)
        {
            infos.push_back(msg);
        }

        void Debug(const std::string& msg)
        {
            debugs.push_back(msg);
        }

        std::vector<std::string> warnings;
        std::vector<std::string> infos;
        std::vector<std::string> debugs;
    } logger;

    auto GetOperationMode() const -> OperationMode
    {
        return operationMode;
    }

    auto IsTimeSyncActive() const -> bool
    {
        return timeSyncActive;
    }

    void ChangeParticipantState(ParticipantState state, std::string)
    {
        publishedStates.push_back(state);
    }

    auto HandleCommunicationReady() -> CallbackResult
    {
        return communicationReadyResult;
    }

    auto HandleStarting() -> bool
    {
        return startingResult;
    }

    auto HandleStop() -> bool
    {
        return stopResult;
    }

    auto HandleShutdown() -> bool
    {
        return shutdownResult;
    }

    auto HandleAbort(ParticipantState state) -> bool
    {
        lastAbortState = state;
        return abortResult;
    }

    void StartTime()
    {
        startTimeCalled = true;
    }

    void StopTime()
    {
        stopTimeCalled = true;
    }

    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
    {
        asyncSubscriptionHandler = std::move(handler);
        asyncSubscriptionHandler();
    }

    void CallAfterAllParticipantsReplied(std::function<void()> handler)
    {
        allParticipantsHandler = std::move(handler);
        allParticipantsHandler();
    }

    void NotifyShutdownInConnection()
    {
        notifyShutdownCalled = true;
    }

    void SetFinalStatePromise()
    {
        finalStatePromiseSet = true;
    }

    auto GetLogger() -> Logger*
    {
        return &logger;
    }

    bool startTimeCalled{false};
    bool stopTimeCalled{false};
    bool notifyShutdownCalled{false};
    bool finalStatePromiseSet{false};
    std::function<void()> asyncSubscriptionHandler;
    std::function<void()> allParticipantsHandler;
};

TEST(Test_LifecycleStateMachine, autonomous_flow_reaches_running)
{
    FakeEffects effects;
    effects.operationMode = OperationMode::Autonomous;

    LifecycleStateMachine<FakeEffects> machine{effects};

    machine.Initialize("init");
    machine.StartAutonomous("autonomous");

    EXPECT_EQ(machine.CurrentState(), LifecycleState::Running);
    ASSERT_EQ(effects.publishedStates.size(), 5u);
    EXPECT_EQ(effects.publishedStates[0], ParticipantState::ServicesCreated);
    EXPECT_EQ(effects.publishedStates[1], ParticipantState::CommunicationInitializing);
    EXPECT_EQ(effects.publishedStates[2], ParticipantState::CommunicationInitialized);
    EXPECT_EQ(effects.publishedStates[3], ParticipantState::ReadyToRun);
    EXPECT_EQ(effects.publishedStates[4], ParticipantState::Running);
}

TEST(Test_LifecycleStateMachine, abort_reports_last_state_before_aborting)
{
    FakeEffects effects;
    LifecycleStateMachine<FakeEffects> machine{effects};

    machine.Initialize("init");
    machine.ServicesCreated("services");
    machine.CommunicationInitialized("comm initialized");
    machine.ReadyToRun("ready");

    ASSERT_EQ(machine.CurrentState(), LifecycleState::Running);

    machine.AbortSimulation("abort");

    EXPECT_EQ(machine.CurrentState(), LifecycleState::Shutdown);
    EXPECT_EQ(effects.lastAbortState, ParticipantState::Running);
    EXPECT_TRUE(effects.notifyShutdownCalled);
    EXPECT_TRUE(effects.finalStatePromiseSet);
}

TEST(Test_LifecycleStateMachine, invalid_pause_transitions_to_error)
{
    FakeEffects effects;
    LifecycleStateMachine<FakeEffects> machine{effects};

    machine.Initialize("init");
    machine.Pause("pause from services created");

    EXPECT_EQ(machine.CurrentState(), LifecycleState::Error);
    ASSERT_FALSE(effects.publishedStates.empty());
    EXPECT_EQ(effects.publishedStates.back(), ParticipantState::Error);
}

} // namespace
