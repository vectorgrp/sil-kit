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
#include "LifecycleService.hpp"
#include "LifecycleManagement.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

// State

void State::InitializeLifecycle(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}
void State::ServicesCreated(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}
void State::CommunicationInitializing(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}
void State::CommunicationInitialized(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}
void State::ReadyToRun(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}
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

void State::StopSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void State::RestartParticipant(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

bool State::ShutdownParticipant(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
    return false;
}

void State::Error(std::string reason)
{
    _lifecycleManager->SetStateError(std::move(reason));
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


void State::ProcessAbortCommand()
{
    std::string reason = "Received SystemCommand::AbortSimulation during callback.";
    _abortRequested = false;
    _lifecycleManager->SetAbortingState(reason);
}

// InvalidState
void InvalidState::InitializeLifecycle(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), std::move(reason));
}

void InvalidState::AbortSimulation()
{
    std::string msg = "Received Abort message before lifecycle was started.";
    _lifecycleManager->GetLogger()->Warn(msg);
    _lifecycleManager->SetStateError(std::move(msg));
}

void InvalidState::ResolveAbortSimulation(std::string)
{
    // NOP -- we are not even initialized yet...
}

std::string InvalidState::toString()
{
    return "Invalid";
}

auto InvalidState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Invalid;
}

// ServicesCreatedState
void ServicesCreatedState::ServicesCreated(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetCommunicationInitializingState(), std::move(reason));
}

void ServicesCreatedState::AbortSimulation()
{
    ResolveAbortSimulation("Received SystemCommand::AbortSimulation.");
}

void ServicesCreatedState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    _lifecycleManager->SetAbortingState(std::move(reason));
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
void CommunicationInitializingState::CommunicationInitializing(std::string reason)
{
        auto advanceToInitializedState =
            [this, reason]() {
				_lifecycleManager->SetState(_lifecycleManager->GetCommunicationInitializedState(), std::move(reason));
            };
        
        if (_lifecycleManager->GetOperationMode() == OperationMode::Coordinated)
        {
            // Delay the next state until pending subscriptions of controllers are completed. 
            // Applies to all controllers that are included in the trait UseAsyncRegistration().
            _lifecycleManager->SetAsyncSubscriptionsCompletionHandler(std::move(advanceToInitializedState));
        }
        else
        {
            // No CommunicationReady guarantees for uncoordinated participants
            advanceToInitializedState();
        }

}

void CommunicationInitializingState::AbortSimulation()
{
    // TODO check if this makes sense while initializing...
    ResolveAbortSimulation("Received SystemCommand::AbortSimulation.");
}

void CommunicationInitializingState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    _lifecycleManager->SetAbortingState(std::move(reason));
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
void CommunicationInitializedState::CommunicationInitialized(std::string reason) 
{
    auto success = _lifecycleManager->HandleCommunicationReady();
    switch (success)
    {
    case SilKit::Services::Orchestration::CallbackResult::Error:
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            // Switch to error state if handle triggers error
            _lifecycleManager->SetStateError("Exception during CommunicationReadyHandle execution.");
        }
        break;
    case SilKit::Services::Orchestration::CallbackResult::Completed:
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            _lifecycleManager->SetState(_lifecycleManager->GetReadyToRunState(), std::move(reason));
        }
        break;
    case SilKit::Services::Orchestration::CallbackResult::Deferred: 
        _lifecycleManager->GetLogger()->Debug("Deferred CommunicationReady callback.");
        break;
    default: break;
    }
}

void CommunicationInitializedState::AbortSimulation()
{
    _abortRequested = true;
}

void CommunicationInitializedState::ResolveAbortSimulation(std::string /*reason*/)
{
    _lifecycleManager->GetLogger()->Warn(
        "Reached CommunicationInitializedState::ResolveAbortSimulation. Discarding call.");
    // NOP 
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
void ReadyToRunState::ReadyToRun(std::string reason) 
{
    // TODO check which cases need actual abort handling
    if (!_lifecycleManager->GetService()->IsTimeSyncActive())
    {
        std::stringstream ss;
        ss << "Participant is about to start running and virtual time synchronization is inactive";

        _handlerExecuting = true;
        auto success = _lifecycleManager->HandleStarting();
        if (success)
        {
            if (_abortRequested)
            {
                ProcessAbortCommand();
            }
            else
            {
                _lifecycleManager->SetState(_lifecycleManager->GetRunningState(),
                                            "Finished StartingHandler execution.");
            }
        }
        else
        {
            if (_abortRequested)
            {
                ProcessAbortCommand();
            }
            else
            {
                // Switch to error state if handle triggers error
                _lifecycleManager->SetStateError("Exception during StartingHandler execution.");
            }
        }
        _handlerExecuting = false;
    }
    else
    {
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            _lifecycleManager->SetState(_lifecycleManager->GetRunningState(), std::move(reason));
        }
    }
}

void ReadyToRunState::AbortSimulation()
{
    if (_handlerExecuting)
    {
        _abortRequested = true;
    }
    else
    {
        // if we are still waiting for a system state update and receive an abort command, execute immediately.
        ResolveAbortSimulation("Received SystemCommand::AbortSimulation.");
    }
}

void ReadyToRunState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    _lifecycleManager->SetAbortingState(reason);
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
void RunningState::RunSimulation(std::string /*reason*/) 
{
    _lifecycleManager->StartRunning();
}

void RunningState::PauseSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetPausedState(), std::move(reason));
}

void RunningState::ContinueSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void RunningState::StopSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppingState(), reason);
    _lifecycleManager->Stop(std::move(reason));
}

void RunningState::AbortSimulation()
{
    // TODO handle abort during executeSimStep
    // For now, just abort and hope for the best...
    ResolveAbortSimulation("Received SystemCommand::AbortSimulation.");
}

void RunningState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetAbortingState(std::move(reason));
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

void PausedState::StopSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetStoppingState(), std::move(reason));
    auto success = _lifecycleManager->HandleStop();
    if (success)
    {
        _lifecycleManager->SetState(_lifecycleManager->GetStoppedState(), "Finished StopHandler execution.");
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
    }
    else
    {
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            // Switch to error state if handle triggers error
            _lifecycleManager->SetStateError("Exception during StopHandler execution.");
        }
    }
}

void PausedState::AbortSimulation()
{
    // TODO handle abort during executeSimStep
    // For now, just abort and hope for the best...
    ResolveAbortSimulation("Received abort simulation.");
}

void PausedState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetAbortingState(std::move(reason));
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
void StoppingState::StopSimulation(std::string reason)
{
    auto success = _lifecycleManager->HandleStop();
    if (success)
    {
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            _lifecycleManager->SetState(_lifecycleManager->GetStoppedState(), std::move(reason));
        }
    }
    else
    {
        if (_abortRequested)
        {
            ProcessAbortCommand();
        }
        else
        {
            // Switch to error state if handle triggers error
            _lifecycleManager->SetStateError("Exception during StopHandler execution.");
        }
    }
}

void StoppingState::AbortSimulation()
{
    _abortRequested = true;
}

void StoppingState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetAbortingState(std::move(reason));
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
void StoppedState::StopSimulation(std::string /*reason*/)
{
    // NOP - stop was already processed (e.g., via a userStop)
}


void StoppedState::RestartParticipant(std::string /*reason*/)
{
    throw std::runtime_error("Restart is currently not supported.");

    //_lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), std::move(reason));
    //_lifecycleManager->Restart(std::move(reason));
}

bool StoppedState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShuttingDownState(), reason);
    return _lifecycleManager->Shutdown(std::move(reason));
}

void StoppedState::AbortSimulation()
{
    ResolveAbortSimulation("Received abort simulation.");
}

void StoppedState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetAbortingState(std::move(reason));
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
bool ShuttingDownState::ShutdownParticipant(std::string reason)
{
    auto success = _lifecycleManager->HandleShutdown();
    if (success)
    {
        // NOP
    }
    else
    {
        std::string msg = "ShutdownHandler threw an exception. This is ignored. The participant will now shut down.";
        _lifecycleManager->GetLogger()->Warn(msg);
    }
    _lifecycleManager->SetState(_lifecycleManager->GetShutdownState(), std::move(reason));
    return true;
}

void ShuttingDownState::AbortSimulation()
{
    _lifecycleManager->GetLogger()->Info("Received abort signal while shutting down - ignoring abort.");
}

void ShuttingDownState::ResolveAbortSimulation(std::string /*reason*/)
{
    _lifecycleManager->GetLogger()->Info("Received abort signal while shutting down - ignoring abort.");
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
void ShutdownState::AbortSimulation()
{
    _lifecycleManager->GetLogger()->Info("Received abort signal after shutdown - ignoring abort.");
}

void ShutdownState::ResolveAbortSimulation(std::string /*reason*/)
{
    _lifecycleManager->GetLogger()->Info("Received abort signal after shutdown - ignoring abort.");
}

auto ShutdownState::toString() -> std::string
{
    return "Shutdown";
}

auto ShutdownState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Shutdown;
}

// AbortingState
bool AbortingState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShutdownState(), std::move(reason));
    return true;
}
void AbortingState::AbortSimulation()
{
    // NOP - we are already in the aborting state
}

void AbortingState::ResolveAbortSimulation(std::string reason)
{
    auto success = _lifecycleManager->HandleAbort();
    if (success)
    {
        // NOP
    }
    else
    {
        std::string msg = "ShutdownHandler threw an exception. This is ignored. The participant will now shut down.";
        _lifecycleManager->GetLogger()->Warn(msg);
    }
    _lifecycleManager->ShutdownAfterAbort(std::move(reason));
}

auto AbortingState::toString() -> std::string
{
    return "Aborting";
}

auto AbortingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Aborting;
}

// ErrorState
void ErrorState::RestartParticipant(std::string /*reason*/)
{
    throw std::runtime_error("Restart feature is currently not supported.");

    //_lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), std::move(reason));
}

void ErrorState::StopSimulation(std::string reason)
{
    _lifecycleManager->Shutdown(std::move(reason));
}

bool ErrorState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShuttingDownState(), std::move(reason));
    _lifecycleManager->Shutdown("Error recovery via shutdown.");
    return true;
}

void ErrorState::AbortSimulation()
{
    ResolveAbortSimulation("Received abort simulation.");
}

void ErrorState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetShuttingDownState(), std::move(reason));
    _lifecycleManager->Shutdown("Error recovery via abort simulation.");
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