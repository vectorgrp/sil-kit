// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "ILifecycleStates.hpp"
#include "LifecycleManagement.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class State : public ILifecycleState
{
public:
    State(LifecycleManagement* lifecycleManager)
        : _lifecycleManager(lifecycleManager)
    {
    }

public:
    virtual ~State() = default;

    // See method description in ILifecycleState

    virtual void Initialize(std::string reason) override;
    virtual void ServicesCreated(std::string reason) override;
    virtual void CommunicationInitializing(std::string reason) override;
    virtual void CommunicationInitialized(std::string reason) override;
    virtual void CompleteCommunicationReadyHandler(std::string reason) override;
    virtual void ReadyToRun(std::string reason) override;
    virtual void RunSimulation(std::string reason) override;
    virtual void PauseSimulation(std::string reason) override;
    virtual void ContinueSimulation(std::string reason) override;
    virtual void StopSimulation(std::string reason) override;
    virtual void RestartParticipant(std::string reason) override;
    virtual void ShutdownParticipant(std::string reason) override;
    virtual void Error(std::string reason) override;

protected:
    void InvalidStateTransition(std::string transitionName, bool triggerErrorState, std::string originalReason);
    bool IsAnyOf(SystemState state, std::initializer_list<SystemState> stateList);

    void ProcessAbortCommandInCallback();
    void ProcessAbortCommand(std::string reason);

protected:
    LifecycleManagement* _lifecycleManager;
    bool _abortRequested{false};
};

class InvalidState : public State
{
public:
    InvalidState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void Initialize(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ServicesCreatedState : public State
{
public:
    ServicesCreatedState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void ServicesCreated(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class CommunicationInitializingState : public State
{
public:
    CommunicationInitializingState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void ServicesCreated(std::string reason) override;
    void CommunicationInitializing(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class CommunicationInitializedState : public State
{
public:
    CommunicationInitializedState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void CommunicationInitializing(std::string reason) override;
    void CommunicationInitialized(std::string reason) override;
    void CompleteCommunicationReadyHandler(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    std::atomic<bool> _handlerExecuting{false};
};

class ReadyToRunState : public State
{
public:
    ReadyToRunState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void CommunicationInitializing(std::string reason) override;
    void CommunicationInitialized(std::string reason) override;
    void ReadyToRun(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    std::atomic<bool> _handlerExecuting{false};
};

class RunningState : public State
{
public:
    RunningState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void CommunicationInitializing(std::string reason) override;
    void CommunicationInitialized(std::string reason) override;
    void ReadyToRun(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopSimulation(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class PausedState : public State
{
public:
    PausedState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopSimulation(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class StoppingState : public State
{
public:
    StoppingState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void StopSimulation(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class StoppedState : public State
{
public:
    StoppedState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void StopSimulation(std::string reason) override;

    void RestartParticipant(std::string reason) override;
    void ShutdownParticipant(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ShuttingDownState : public State
{
public:
    ShuttingDownState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void StopSimulation(std::string reason) override;

    void ShutdownParticipant(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ShutdownState : public State
{
public:
    ShutdownState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }
    void Initialize(std::string reason) override;

    void CommunicationInitialized(std::string reason) override;
    void ReadyToRun(std::string reason) override;
    void StopSimulation(std::string reason) override;
    void Error(std::string reason) override;

    void ShutdownParticipant(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class AbortingState : public State
{
public:
    AbortingState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }
    void Initialize(std::string reason) override;

    void ShutdownParticipant(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ErrorState : public State
{
public:
    ErrorState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void Initialize(std::string reason) override;

    void StopSimulation(std::string reason) override;
    void RestartParticipant(std::string reason) override;
    void ShutdownParticipant(std::string reason) override;
    void Error(std::string reason) override;

    void AbortSimulation(std::string reason) override;
    void ResolveAbortSimulation(std::string reason) override;
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};
} // namespace Orchestration
} // namespace Services
} // namespace SilKit
