// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

#include "silkit/core/sync/SyncDatatypes.hpp"

#include "ILifecycleStates.hpp"
#include "LifecycleManagement.hpp"

namespace SilKit {
namespace Core {
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

    virtual void RunSimulation(std::string reason) override;
    virtual void PauseSimulation(std::string reason) override;
    virtual void ContinueSimulation(std::string reason) override;

    virtual void StopNotifyUser(std::string reason) override;
    virtual void StopHandlerDone(std::string reason) override;

    virtual void Restart(std::string reason) override;

    virtual void ShutdownNotifyUser(std::string reason) override;
    virtual void ShutdownHandlerDone(std::string reason) override;

    virtual void AbortSimulation(std::string reason) override;
    virtual void Error(std::string reason) override;

    virtual void NewSystemState(SystemState systemState) override;

protected:
    void InvalidStateTransition(std::string transitionName, bool triggerErrorState, std::string originalReason);
    bool IsAnyOf(SystemState state, std::initializer_list<SystemState> stateList);

protected:
    LifecycleManagement* _lifecycleManager;
};
    
class InvalidState : public State
{
public:
    InvalidState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

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

    void NewSystemState(SystemState systemState) override;

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

    void NewSystemState(SystemState systemState) override;

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

    void NewSystemState(SystemState systemState) override;

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ReadyToRunState : public State
{
public:
    ReadyToRunState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
        , _isSystemReadyToRun(false)
        , _receivedRunCommand(false)
    {
    }

    void NewSystemState(SystemState systemState) override;

    void RunSimulation(std::string reason) override;
    void AbortSimulation(std::string reason) override;

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    bool _isSystemReadyToRun;
    bool _receivedRunCommand;
};

class RunningState : public State
{
public:
    RunningState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void RunSimulation(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;

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
    void StopNotifyUser(std::string reason) override;

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

    void StopNotifyUser(std::string reason) override;
    void StopHandlerDone(std::string reason) override;
    void AbortSimulation(std::string reason) override;

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    bool _abortRequested = false;
};

class StoppedState : public State
{
public:
    StoppedState(LifecycleManagement* lifecycleManager)
        : State(lifecycleManager)
    {
    }

    void StopNotifyUser(std::string reason) override;
    void StopHandlerDone(std::string reason) override;

    void Restart(std::string reason) override;

    void ShutdownNotifyUser(std::string reason) override;

    void AbortSimulation(std::string reason) override;

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

    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandlerDone(std::string reason) override;

    void AbortSimulation(std::string reason) override;

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

    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandlerDone(std::string reason) override;

    void AbortSimulation(std::string reason) override;

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

    void RunSimulation(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    void StopHandlerDone(std::string reason) override;
    void Restart(std::string reason) override;
    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandlerDone(std::string reason) override;
    void AbortSimulation(std::string reason) override;
    void Error(std::string reason) override;

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};
} // namespace Orchestration
} // namespace Core
} // namespace SilKit
