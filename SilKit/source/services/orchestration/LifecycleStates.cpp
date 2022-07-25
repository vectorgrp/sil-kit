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

#include "LifecycleStates.hpp"
#include "LifecycleManagement.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

// State
void State::RunSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::PauseSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ContinueSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::StopNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::StopHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::Restart(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::AbortSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppedState(), reason);
    _lifecycleManager->Shutdown(std::move(reason));
}

void State::Error(std::string reason)
{
    _lifecycleManager->SetStateError(std::move(reason));
}

void State::NewSystemState(SystemState systemState)
{
    std::stringstream ss;
    ss << toString() << " received SystemState::" << systemState;
    _lifecycleManager->GetLogger()->Info(ss.str());
}

void State::InvalidStateTransition(std::string transitionName, bool triggerErrorState, std::string originalReason)
{
    std::stringstream ss;
    ss << "Detected invalid state transition.\n"
       << "Current state: " << toString() << "\n"
       << "Requested transition: " << transitionName << "\n"
       << "Original reason: " << originalReason;

    if (triggerErrorState)
    {
        _lifecycleManager->Error(ss.str());
    }
    else
    {
        _lifecycleManager->GetLogger()->Warn(ss.str());
    }
}

bool State::IsAnyOf(SystemState state, std::initializer_list<SystemState> stateList)
{
    return std::any_of(begin(stateList), end(stateList), [=](auto candidate) {
        return candidate == state;
    });
}

// InvalidState
std::string InvalidState::toString()
{
    return "Invalid";
}

auto InvalidState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Invalid;
}

// ServicesCreatedState
void ServicesCreatedState::NewSystemState(SystemState systemState)
{
    if (IsAnyOf(systemState, {SystemState::ServicesCreated, SystemState::CommunicationInitializing,
                              SystemState::CommunicationInitialized, SystemState::ReadyToRun, SystemState::Running}))
    {
        std::stringstream ss;
        ss << "Received SystemState::" << systemState;
        _lifecycleManager->SetState(_lifecycleManager->GetCommunicationInitializingState(), ss.str());
    }
}

auto ServicesCreatedState::toString() -> std::string
{
    return "ServicesCreated";
}

auto ServicesCreatedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::ServicesCreated;
}

// CommunicationInitializingState
void CommunicationInitializingState::NewSystemState(SystemState systemState)
{
    if (IsAnyOf(systemState, {SystemState::CommunicationInitializing, SystemState::CommunicationInitialized,
                              SystemState::ReadyToRun, SystemState::Running}))
    {
        // TODO Resolve waitforme
        // 1. TODO set shared_future
        // 2. TODO await all queued futures
        std::stringstream ss;
        ss << "Received SystemState::" << systemState;
        _lifecycleManager->SetState(_lifecycleManager->GetCommunicationInitializedState(), ss.str());
    }
    else
    {
        _lifecycleManager->GetLogger()->Warn("Received illegal new system state in state '{}'", this->toString());
    }
}

auto CommunicationInitializingState::toString() -> std::string
{
    return "CommunicationInitializing";
}

auto CommunicationInitializingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::CommunicationInitializing;
}

// CommunicationInitializedState
void CommunicationInitializedState::NewSystemState(SystemState systemState)
{
    if (IsAnyOf(systemState, {SystemState::CommunicationInitialized, SystemState::ReadyToRun, SystemState::Running}))
    {
        std::stringstream ss;
        ss << "Received SystemState::" << systemState;
        // Next state is set by context (Error or Initialized)
        _lifecycleManager->HandleCommunicationReady(ss.str());
    }
}

auto CommunicationInitializedState::toString() -> std::string
{
    return "CommunicationInitialized";
}

auto CommunicationInitializedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::CommunicationInitialized;
}

// ReadyToRunState
void ReadyToRunState::NewSystemState(SystemState systemState)
{
    if (IsAnyOf(systemState, {SystemState::ReadyToRun, SystemState::Running}))
    {
        if (_receivedRunCommand)
        {
            std::stringstream ss;
            ss << "Received SystemCommand::Run and SystemState::" << systemState;
            _receivedRunCommand = false;
            _lifecycleManager->SetState(_lifecycleManager->GetRunningState(), ss.str());
        }
        else
        {
            _isSystemReadyToRun = true;
        }
    }
    else
    {
        _isSystemReadyToRun = false;
    }
}

void ReadyToRunState::RunSimulation(std::string reason)
{
    if (_isSystemReadyToRun)
    {
        _isSystemReadyToRun = false;

        if (!_lifecycleManager->GetService()->IsTimeSyncActive())
        {
            _lifecycleManager->HandleStarting(std::move(reason));
            // state transition handled by lifecycle manager - could be running or error
            return;
        }

        _lifecycleManager->SetState(_lifecycleManager->GetRunningState(), std::move(reason));
        return;
    }
    else
    {
        _receivedRunCommand = true;
        return;
    }
}

void ReadyToRunState::AbortSimulation(std::string reason)
{
    _receivedRunCommand = false;
    _isSystemReadyToRun = false;
    State::AbortSimulation(std::move(reason));
}

std::string ReadyToRunState::toString()
{
    return "ReadyToRun";
}

auto ReadyToRunState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::ReadyToRun;
}

// RunningState
void RunningState::RunSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void RunningState::PauseSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetPausedState(), std::move(reason));
}

void RunningState::ContinueSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void RunningState::StopNotifyUser(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppingState(), reason);
    // Context will set next state
    _lifecycleManager->HandleStop(std::move(reason));
}

auto RunningState::toString() -> std::string
{
    return "Running";
}

auto RunningState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Running;
}

// PausedState
void PausedState::PauseSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void PausedState::ContinueSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetRunningState(), std::move(reason));
}

void PausedState::StopNotifyUser(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppingState(), std::move(reason));
    // Context will set next state
    _lifecycleManager->HandleStop(std::move(reason));
}

auto PausedState::toString() -> std::string
{
    return "Paused";
}

auto PausedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Paused;
}

// StoppingState
void StoppingState::StopNotifyUser(std::string reason)
{
    // NOP
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppingState::StopHandlerDone(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppedState(), std::move(reason));
    if (_abortRequested)
    {
        _abortRequested = false;
        _lifecycleManager->Shutdown("Received SystemCommand::AbortSimulation during callback.");
    }
}

void StoppingState::AbortSimulation(std::string reason)
{
    _abortRequested = true;
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

auto StoppingState::toString() -> std::string
{
    return "Stopping";
}

auto StoppingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Stopping;
}

// StoppedState
void StoppedState::StopNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppedState::StopHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppedState::Restart(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), reason);
}

void StoppedState::ShutdownNotifyUser(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShuttingDownState(), reason);
    _lifecycleManager->HandleShutdown(std::move(reason));
}

void StoppedState::AbortSimulation(std::string reason)
{
    _lifecycleManager->Shutdown(std::move(reason));
}

auto StoppedState::toString() -> std::string
{
    return "Stopped";
}

auto StoppedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Stopped;
}

// ShuttingDownState
void ShuttingDownState::ShutdownNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ShuttingDownState::ShutdownHandlerDone(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShutdownState(), std::move(reason));
}

void ShuttingDownState::AbortSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

auto ShuttingDownState::toString() -> std::string
{
    return "ShuttingDown";
}

auto ShuttingDownState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::ShuttingDown;
}

// ShutdownState
void ShutdownState::ShutdownNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ShutdownState::ShutdownHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ShutdownState::AbortSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

auto ShutdownState::toString() -> std::string
{
    return "Shutdown";
}

auto ShutdownState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Shutdown;
}

// ErrorState
void ErrorState::RunSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::PauseSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::ContinueSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::StopNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::StopHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::Restart(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), std::move(reason));
}

void ErrorState::ShutdownNotifyUser(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShuttingDownState(), std::move(reason));
    _lifecycleManager->HandleShutdown(std::move(reason));
}

void ErrorState::ShutdownHandlerDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::AbortSimulation(std::string reason)
{
    _lifecycleManager->Shutdown(std::move(reason));
}

void ErrorState::Error(std::string reason)
{
    _lifecycleManager->GetLogger()->Warn("Received error transition within error state. Original reason: "
                                         + std::move(reason));
}

auto ErrorState::toString() -> std::string
{
    return "Error";
}

auto ErrorState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Error;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit