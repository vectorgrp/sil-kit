/* Copyright (c) 2022 Vector Informatik GmbH

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

#pragma once

#include <string>

#include "silkit/services/orchestration/SyncDatatypes.hpp"

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
} // namespace Services
} // namespace SilKit
