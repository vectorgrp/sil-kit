// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/participant/exception.hpp"

#include "services/orchestration/LifecycleManagement.hpp"
#include "services/orchestration/LifecycleService.hpp"
#include "core/mock/participant/MockParticipant.hpp"

// These tests drive the lifecycle states directly. They cover transitions that the public LifecycleService API
// filters out before they reach the state machine (e.g., Pause() in a non-running state), as well as transitions
// that only occur during races between SystemState updates and user calls.

namespace {

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core::Tests;
using namespace SilKit::Services::Orchestration;

using StateGetter = ILifecycleState* (LifecycleManagement::*)();
using Transition = void (ILifecycleState::*)(std::string);

struct TransitionCase
{
    std::string name;
    StateGetter from;
    Transition transition;
    StateGetter expected;
};

// Records the ParticipantStates published by the lifecycle service, i.e., the sequence of entered states.
class RecordingParticipant : public DummyParticipant
{
public:
    using DummyParticipant::SendMsg;
    void SendMsg(const SilKit::Core::IServiceEndpoint* /*from*/, const ParticipantStatus& msg) override
    {
        enteredStates.push_back(msg.state);
    }

    std::vector<ParticipantState> enteredStates;
};

class Test_LifecycleStates : public testing::Test
{
protected:
    Test_LifecycleStates()
    {
        lifecycleService.SetLifecycleConfiguration({OperationMode::Coordinated});
    }

    // Enter the given state and forget the states entered so far
    void SetUpState(ILifecycleState* state)
    {
        mgmt.SetState(state, "Test setup");
        participant.enteredStates.clear();
    }

    void RunTransitionCases(const std::vector<TransitionCase>& cases)
    {
        for (const auto& tc : cases)
        {
            SCOPED_TRACE(tc.name);
            auto* fromState = (mgmt.*tc.from)();
            mgmt.SetState(fromState, "Test setup");
            (fromState->*tc.transition)("Test transition");
            EXPECT_EQ(mgmt.GetCurrentState(), (mgmt.*tc.expected)())
                << "current state is " << mgmt.GetCurrentState()->toString();
        }
    }

protected:
    NiceMock<RecordingParticipant> participant;
    LifecycleService lifecycleService{&participant};
    LifecycleManagement mgmt{&participant, participant.GetLoggerInternal(), &lifecycleService};
};

// Selected invalid state transitions move the participant to the Error state. This is not the full matrix of
// invalid transitions: the cases cover transitions that are not reached via the public API or other tests. See
// restart_transitions for one transition checked in every state.
TEST_F(Test_LifecycleStates, invalid_transitions_lead_to_error_state)
{
    using LM = LifecycleManagement;
    using LS = ILifecycleState;
    RunTransitionCases({
        {"ServicesCreated: CommunicationInitializing", &LM::GetServicesCreatedState, &LS::CommunicationInitializing,
         &LM::GetErrorState},
        {"ServicesCreated: CommunicationInitialized", &LM::GetServicesCreatedState, &LS::CommunicationInitialized,
         &LM::GetErrorState},
        {"ServicesCreated: ReadyToRun", &LM::GetServicesCreatedState, &LS::ReadyToRun, &LM::GetErrorState},
        {"ServicesCreated: PauseSimulation", &LM::GetServicesCreatedState, &LS::PauseSimulation, &LM::GetErrorState},
        {"ServicesCreated: ContinueSimulation", &LM::GetServicesCreatedState, &LS::ContinueSimulation,
         &LM::GetErrorState},
        {"Running: Initialize", &LM::GetRunningState, &LS::Initialize, &LM::GetErrorState},
        {"Running: CompleteCommunicationReadyHandler", &LM::GetRunningState, &LS::CompleteCommunicationReadyHandler,
         &LM::GetErrorState},
        {"Running: ShutdownParticipant", &LM::GetRunningState, &LS::ShutdownParticipant, &LM::GetErrorState},
        {"Running: RestartParticipant", &LM::GetRunningState, &LS::RestartParticipant, &LM::GetErrorState},
        {"Paused: PauseSimulation", &LM::GetPausedState, &LS::PauseSimulation, &LM::GetErrorState},
    });
}

// Transitions that are only warned about or ignored on purpose (e.g., due to races with SystemState updates)
// leave the state unchanged.
TEST_F(Test_LifecycleStates, tolerated_and_ignored_transitions_keep_state)
{
    using LM = LifecycleManagement;
    using LS = ILifecycleState;
    RunTransitionCases({
        // Invalid, but only warned about
        {"ServicesCreated: StopSimulation", &LM::GetServicesCreatedState, &LS::StopSimulation,
         &LM::GetServicesCreatedState},
        {"Running: ContinueSimulation", &LM::GetRunningState, &LS::ContinueSimulation, &LM::GetRunningState},
        // Concurrent SystemState transitions
        {"ReadyToRun: CommunicationInitializing", &LM::GetReadyToRunState, &LS::CommunicationInitializing,
         &LM::GetReadyToRunState},
        {"Running: CommunicationInitializing", &LM::GetRunningState, &LS::CommunicationInitializing,
         &LM::GetRunningState},
        {"Running: CommunicationInitialized", &LM::GetRunningState, &LS::CommunicationInitialized,
         &LM::GetRunningState},
        // Stop and abort during shutdown
        {"ShuttingDown: StopSimulation", &LM::GetShuttingDownState, &LS::StopSimulation, &LM::GetShuttingDownState},
        {"ShuttingDown: AbortSimulation", &LM::GetShuttingDownState, &LS::AbortSimulation,
         &LM::GetShuttingDownState},
        {"Shutdown: Initialize", &LM::GetShutdownState, &LS::Initialize, &LM::GetShutdownState},
        {"Shutdown: CommunicationInitialized", &LM::GetShutdownState, &LS::CommunicationInitialized,
         &LM::GetShutdownState},
        {"Shutdown: ReadyToRun", &LM::GetShutdownState, &LS::ReadyToRun, &LM::GetShutdownState},
        {"Shutdown: StopSimulation", &LM::GetShutdownState, &LS::StopSimulation, &LM::GetShutdownState},
        {"Shutdown: AbortSimulation", &LM::GetShutdownState, &LS::AbortSimulation, &LM::GetShutdownState},
        {"Shutdown: Error", &LM::GetShutdownState, &LS::Error, &LM::GetShutdownState},
        // Initialize after an invalid workflow configuration led to Error/Aborting
        {"Aborting: Initialize", &LM::GetAbortingState, &LS::Initialize, &LM::GetAbortingState},
        {"Aborting: AbortSimulation", &LM::GetAbortingState, &LS::AbortSimulation, &LM::GetAbortingState},
        {"Error: Initialize", &LM::GetErrorState, &LS::Initialize, &LM::GetErrorState},
        {"Error: Error", &LM::GetErrorState, &LS::Error, &LM::GetErrorState},
    });
}

// An abort during CommunicationInitializing calls the abort handler and ends in Shutdown.
TEST_F(Test_LifecycleStates, abort_during_communication_initializing_shuts_down)
{
    std::vector<ParticipantState> abortHandlerStates;
    lifecycleService.SetAbortHandler([&abortHandlerStates](auto state) { abortHandlerStates.push_back(state); });

    SetUpState(mgmt.GetCommunicationInitializingState());
    mgmt.AbortSimulation("Test transition");

    EXPECT_EQ(mgmt.GetCurrentState(), mgmt.GetShutdownState());
    EXPECT_THAT(abortHandlerStates, ElementsAre(ParticipantState::CommunicationInitializing));
    // Skips ShuttingDown, Stopping and Stopped
    EXPECT_THAT(participant.enteredStates, ElementsAre(ParticipantState::Aborting, ParticipantState::Shutdown));
}

// Stop in the Error state calls the shutdown handler and ends in Shutdown.
TEST_F(Test_LifecycleStates, stop_in_error_state_shuts_down)
{
    int shutdownHandlerCalls = 0;
    lifecycleService.SetShutdownHandler([&shutdownHandlerCalls] { ++shutdownHandlerCalls; });

    SetUpState(mgmt.GetErrorState());
    mgmt.Stop("Test transition");

    EXPECT_EQ(mgmt.GetCurrentState(), mgmt.GetShutdownState());
    EXPECT_EQ(shutdownHandlerCalls, 1);
    // Skips Stopping and Stopped
    EXPECT_THAT(participant.enteredStates, ElementsAre(ParticipantState::ShuttingDown, ParticipantState::Shutdown));
}

// Restarting a participant is not supported: it throws in Stopped and Error, where a restart would be valid. In all
// other states, it is an invalid transition to Error, except for Shutdown, which ignores errors.
TEST_F(Test_LifecycleStates, restart_transitions)
{
    using LM = LifecycleManagement;
    const std::vector<StateGetter> toErrorStates{
        &LM::GetInvalidState,
        &LM::GetServicesCreatedState,
        &LM::GetCommunicationInitializingState,
        &LM::GetCommunicationInitializedState,
        &LM::GetReadyToRunState,
        &LM::GetRunningState,
        &LM::GetPausedState,
        &LM::GetStoppingState,
        &LM::GetShuttingDownState,
        &LM::GetAbortingState,
    };
    for (const auto& getState : toErrorStates)
    {
        auto* state = (mgmt.*getState)();
        SCOPED_TRACE(state->toString());
        SetUpState(state);
        mgmt.Restart("Test transition");
        EXPECT_EQ(mgmt.GetCurrentState(), mgmt.GetErrorState());
    }

    SetUpState(mgmt.GetShutdownState());
    mgmt.Restart("Test transition");
    EXPECT_EQ(mgmt.GetCurrentState(), mgmt.GetShutdownState());

    for (const auto& getState : {&LM::GetStoppedState, &LM::GetErrorState})
    {
        auto* state = (mgmt.*getState)();
        SCOPED_TRACE(state->toString());
        SetUpState(state);
        EXPECT_THROW(mgmt.Restart("Test transition"), SilKitError);
        EXPECT_EQ(mgmt.GetCurrentState(), state);
    }
}

// Each lifecycle state has the expected name and ParticipantState.
TEST_F(Test_LifecycleStates, state_names_match_participant_states)
{
    struct NameCase
    {
        StateGetter state;
        std::string name;
        ParticipantState participantState;
    };

    using LM = LifecycleManagement;
    const std::vector<NameCase> cases{
        {&LM::GetInvalidState, "Invalid", ParticipantState::Invalid},
        {&LM::GetServicesCreatedState, "ServicesCreated", ParticipantState::ServicesCreated},
        {&LM::GetCommunicationInitializingState, "CommunicationInitializing",
         ParticipantState::CommunicationInitializing},
        {&LM::GetCommunicationInitializedState, "CommunicationInitialized", ParticipantState::CommunicationInitialized},
        {&LM::GetReadyToRunState, "ReadyToRun", ParticipantState::ReadyToRun},
        {&LM::GetRunningState, "Running", ParticipantState::Running},
        {&LM::GetPausedState, "Paused", ParticipantState::Paused},
        {&LM::GetStoppingState, "Stopping", ParticipantState::Stopping},
        {&LM::GetStoppedState, "Stopped", ParticipantState::Stopped},
        {&LM::GetShuttingDownState, "ShuttingDown", ParticipantState::ShuttingDown},
        {&LM::GetShutdownState, "Shutdown", ParticipantState::Shutdown},
        {&LM::GetAbortingState, "Aborting", ParticipantState::Aborting},
        {&LM::GetErrorState, "Error", ParticipantState::Error},
    };

    for (const auto& nc : cases)
    {
        SCOPED_TRACE(nc.name);
        auto* state = (mgmt.*nc.state)();
        EXPECT_EQ(state->toString(), nc.name);
        EXPECT_EQ(state->GetParticipantState(), nc.participantState);
    }
}

} // anonymous namespace
