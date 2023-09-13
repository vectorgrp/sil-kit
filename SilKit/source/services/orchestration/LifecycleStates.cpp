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

#include <fmt/format.h>

#include "ILogger.hpp"
#include "LifecycleStates.hpp"
#include "LifecycleService.hpp"
#include "LifecycleManagement.hpp"
#include "IRequestReplyService.hpp"
#include "procs/IParticipantReplies.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

// ------------------------------------
// State
// ------------------------------------

void State::Initialize(std::string reason)
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

void State::CompleteCommunicationReadyHandler(std::string reason)
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

void State::ShutdownParticipant(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::Error(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetErrorState(), std::move(reason));
}

void State::InvalidStateTransition(std::string transitionName, bool triggerErrorState, std::string originalReason)
{
    auto currentState = toString();
    std::stringstream ss;
    ss << "Detected invalid state transition.\n"
       << "Current state: " << currentState << "\n"
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

void State::ProcessAbortCommandInCallback()
{
    auto reason = fmt::format("Received SystemCommand::AbortSimulation during {} callback", toString());
    ProcessAbortCommand(std::move(reason));
}

void State::ProcessAbortCommand(std::string reason)
{
    _abortRequested = false;
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetAbortingState(),
                                                &ILifecycleState::ResolveAbortSimulation,
                                                std::move(reason));
}

// ------------------------------------
// InvalidState
// ------------------------------------

void InvalidState::Initialize(std::string reason)
{
    _lifecycleManager->SetState(_lifecycleManager->GetServicesCreatedState(), std::move(reason));
}

void InvalidState::AbortSimulation(std::string reason)
{
    ResolveAbortSimulation(std::move(reason));
}

void InvalidState::ResolveAbortSimulation(std::string reason)
{
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetAbortingState(),
                                                &ILifecycleState::ResolveAbortSimulation, std::move(reason));
}

std::string InvalidState::toString()
{
    return "Invalid";
}

auto InvalidState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Invalid;
}

// ------------------------------------
// ServicesCreatedState
// ------------------------------------

void ServicesCreatedState::ServicesCreated(std::string reason)
{
    // ServicesCreated will only advance to CommunicationInitializing after receiving replies from all participants.
    // This guarantees that the ServiceDiscoveryEvents sent before all have arrived and internal pubsub/rpc controllers
    // have been created. Then, the state machine will only advance to CommunicationInitialized after all pending
    // subscriptions have been received.
    _lifecycleManager->GetParticipant()->GetParticipantRepliesProcedure()->CallAfterAllParticipantsReplied(
        [reason, this]() {
            // If done, move forward to CommunicationInitializingState and call ServicesCreated
            _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetCommunicationInitializingState(),
                                                 &ILifecycleState::ServicesCreated, reason);
    });
}

void ServicesCreatedState::AbortSimulation(std::string reason)
{
    ResolveAbortSimulation(std::move(reason));
}

void ServicesCreatedState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetAbortingState(), &ILifecycleState::ResolveAbortSimulation,
                                         std::move(reason));
}

auto ServicesCreatedState::toString() -> std::string
{
    return "ServicesCreated";
}

auto ServicesCreatedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::ServicesCreated;
}

// ------------------------------------
// CommunicationInitializingState
// ------------------------------------

void CommunicationInitializingState::ServicesCreated(std::string reason)
{
    _lifecycleManager->CommunicationInitializing(std::move(reason));
}

void CommunicationInitializingState::CommunicationInitializing(std::string reason)
{
    // Delay the next state until pending subscriptions of controllers are completed.
    // Applies to all controllers that are included in the trait UseAsyncRegistration().
    _lifecycleManager->SetAsyncSubscriptionsCompletionHandler([reason, this]() {
        // If done, move forward to CommunicationInitializedState and call CommunicationInitializing
        _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetCommunicationInitializedState(),
                                             &ILifecycleState::CommunicationInitializing, reason);
    });
}

void CommunicationInitializingState::AbortSimulation(std::string reason)
{
    ResolveAbortSimulation(std::move(reason));
}

void CommunicationInitializingState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetAbortingState(), &ILifecycleState::ResolveAbortSimulation,
                                         std::move(reason));
}

auto CommunicationInitializingState::toString() -> std::string
{
    return "CommunicationInitializing";
}

auto CommunicationInitializingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::CommunicationInitializing;
}

// ------------------------------------
// CommunicationInitializedState
// ------------------------------------

void CommunicationInitializedState::CommunicationInitializing(std::string /*reason*/)
{
    // Autonomous advance, coordinated wait for SystemState Change
    if (_lifecycleManager->GetOperationMode() == OperationMode::Autonomous)
    {
        // Trigger the CommunicationReadyHandler (which can be completed asynchronously)
        _lifecycleManager->CommunicationInitialized("CommunicationInitialized for autonomous participant.");
    }
}

void CommunicationInitializedState::CommunicationInitialized(std::string reason) 
{
    _handlerExecuting = true;
    auto callbackResult = _lifecycleManager->HandleCommunicationReady();
    switch (callbackResult)
    {
    case SilKit::Services::Orchestration::CallbackResult::Error:
        if (_abortRequested)
        {
            ProcessAbortCommandInCallback();
        }
        else
        {
            // Switch to error state if handle triggers error
            _lifecycleManager->SetState(_lifecycleManager->GetErrorState(), "Exception during CommunicationReadyHandle execution.");
        }
        break;
    case SilKit::Services::Orchestration::CallbackResult::Completed:
        if (_abortRequested)
        {
            ProcessAbortCommandInCallback();
        }
        else
        {
            CompleteCommunicationReadyHandler(std::move(reason));
        }
        break;
    case SilKit::Services::Orchestration::CallbackResult::Deferred: 
        _lifecycleManager->GetLogger()->Debug("Deferred CommunicationReady callback.");
        break;
    default: break;
    }
    _handlerExecuting = false;
}

void CommunicationInitializedState::CompleteCommunicationReadyHandler(std::string reason)
{
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetReadyToRunState(), &ILifecycleState::CommunicationInitialized,
                                         reason);
}


void CommunicationInitializedState::AbortSimulation(std::string /*reason*/)
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

void CommunicationInitializedState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
}

auto CommunicationInitializedState::toString() -> std::string
{
    return "CommunicationInitialized";
}

auto CommunicationInitializedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::CommunicationInitialized;
}

// ------------------------------------
// ReadyToRunState
// ------------------------------------

void ReadyToRunState::CommunicationInitializing(std::string /*reason*/)
{
    // NOP for possible concurrent SystemState transition
}

void ReadyToRunState::CommunicationInitialized(std::string /*reason*/)
{
    if (_lifecycleManager->GetOperationMode() == OperationMode::Autonomous)
    {
        _lifecycleManager->ReadyToRun("ReadyToRun for autonomous participant.");
    }
}

void ReadyToRunState::ReadyToRun(std::string reason) 
{
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
                ProcessAbortCommandInCallback();
            }
            else
            {
                _lifecycleManager->SetState(_lifecycleManager->GetRunningState(), "Finished StartingHandler execution.");
            }
        }
        else
        {
            if (_abortRequested)
            {
                ProcessAbortCommandInCallback();
            }
            else
            {
                // Switch to error state if handle triggers error
                _lifecycleManager->SetState(_lifecycleManager->GetErrorState(), "Exception during StartingHandler execution.");
            }
        }
        _handlerExecuting = false;
    }
    else
    {
        if (_abortRequested)
        {
            ProcessAbortCommandInCallback();
        }
        else
        {
            _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetRunningState(), &ILifecycleState::ReadyToRun,
                                                 std::move(reason));
        }
    }
}

void ReadyToRunState::AbortSimulation(std::string reason)
{
    if (_handlerExecuting)
    {
        _abortRequested = true;
    }
    else
    {
        // if we are still waiting for a system state update and receive an abort command, execute immediately.
        ResolveAbortSimulation(std::move(reason));
    }
}

void ReadyToRunState::ResolveAbortSimulation(std::string reason)
{
    // Skip stopping as the simulation was not running yet
    ProcessAbortCommand(std::move(reason));
}

std::string ReadyToRunState::toString()
{
    return "ReadyToRun";
}

auto ReadyToRunState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::ReadyToRun;
}

// ------------------------------------
// RunningState
// ------------------------------------

void RunningState::CommunicationInitializing(std::string /*reason*/)
{
    // NOP for possible concurrent SystemState transition
}

void RunningState::CommunicationInitialized(std::string /*reason*/)
{
    // NOP for possible concurrent SystemState transition
}

void RunningState::ReadyToRun(std::string /*reason*/)
{
    _lifecycleManager->StartTime();
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
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetStoppingState(), &ILifecycleState::StopSimulation,
                                         std::move(reason));
}

void RunningState::AbortSimulation(std::string reason)
{
    // TODO handle abort during executeSimStep
    // For now, just abort and hope for the best...
    ResolveAbortSimulation(std::move(reason));
}

void RunningState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
}

auto RunningState::toString() -> std::string
{
    return "Running";
}

auto RunningState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Running;
}

// ------------------------------------
// PausedState
// ------------------------------------

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
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetStoppingState(), &ILifecycleState::StopSimulation,
                                         std::move(reason));
}

void PausedState::AbortSimulation(std::string /*reason*/)
{
    // TODO handle abort during executeSimStep
    // For now, just abort and hope for the best...
    _lifecycleManager->SetPausePromise();
    ResolveAbortSimulation("Received abort simulation.");
}

void PausedState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
}

auto PausedState::toString() -> std::string
{
    return "Paused";
}

auto PausedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Paused;
}

// ------------------------------------
// StoppingState
// ------------------------------------

void StoppingState::StopSimulation(std::string reason)
{
    auto success = _lifecycleManager->HandleStop();
    if (success)
    {
        if (_abortRequested)
        {
            ProcessAbortCommandInCallback();
        }
        else
        {
            _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetStoppedState(), &ILifecycleState::StopSimulation,
                                                 std::move(reason));
        }
    }
    else
    {
        if (_abortRequested)
        {
            ProcessAbortCommandInCallback();
        }
        else
        {
            // Switch to error state if handle triggers error
            _lifecycleManager->SetState(_lifecycleManager->GetErrorState(), "Exception during StopHandler execution.");
        }
    }
}

void StoppingState::AbortSimulation(std::string /*reason*/)
{
    _abortRequested = true;
}

void StoppingState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
}

auto StoppingState::toString() -> std::string
{
    return "Stopping";
}

auto StoppingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Stopping;
}

// ------------------------------------
// StoppedState
// ------------------------------------

void StoppedState::StopSimulation(std::string reason)
{
    // StoppedState will only advance to ShuttingDown after receiving replies from all participants.
    // This guarantees that the ParticipantState::Stopping has arrived and other participants will 
    // evaluate the correct SystemState and stop themselves.
    _lifecycleManager->GetParticipant()->GetParticipantRepliesProcedure()->CallAfterAllParticipantsReplied(
        [this, reason]() {
            _lifecycleManager->Shutdown(reason);
    });
}

void StoppedState::RestartParticipant(std::string /*reason*/)
{
    throw SilKitError("Restart is currently not supported.");
}

void StoppedState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetShuttingDownState(), &ILifecycleState::ShutdownParticipant,
                                         std::move(reason));
}

void StoppedState::AbortSimulation(std::string reason)
{
    ResolveAbortSimulation(std::move(reason));
}

void StoppedState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
}

auto StoppedState::toString() -> std::string
{
    return "Stopped";
}

auto StoppedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Stopped;
}

// ------------------------------------
// ShuttingDownState
// ------------------------------------

void ShuttingDownState::StopSimulation(std::string /*reason*/)
{
    // Ignore Stop() in ShuttingDownState
}

void ShuttingDownState::ShutdownParticipant(std::string reason)
{
    auto success = _lifecycleManager->HandleShutdown();
    if (!success)
    {
        _lifecycleManager->GetLogger()->Warn(
            "ShutdownHandler threw an exception. This is ignored. The participant will now shut down.");
    }
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetShutdownState(),
                                                &ILifecycleState::ShutdownParticipant,
                                                std::move(reason));
}

void ShuttingDownState::AbortSimulation(std::string /*reason*/)
{
    ResolveAbortSimulation(std::string());
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

// ------------------------------------
// ShutdownState
// ------------------------------------

void ShutdownState::Initialize(std::string /*reason*/)
{
    // Ignore due to possible invalid transition between InvalidWorkflowConfig->Error->Abort->Shutdown and StartLifecycle->Initialize
}

void ShutdownState::ReadyToRun(std::string /*reason*/)
{
    // Ignore due to possible race between SystemState update and AbortSimulation
}

void ShutdownState::CommunicationInitialized(std::string /*reason*/)
{
    // Ignore due to possible race between SystemState update and AbortSimulation
}

void ShutdownState::StopSimulation(std::string /*reason*/)
{
    // Ignore Stop() in ShutdownState
}

void ShutdownState::Error(std::string /*reason*/)
{
    // Ignore Error() in ShutdownState
}

void ShutdownState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->GetParticipant()->GetParticipantRepliesProcedure()->CallAfterAllParticipantsReplied(
        [this, reason]() {
            bool success = _lifecycleManager->GetCurrentState() != _lifecycleManager->GetErrorState();
            if (success)
            {
                _lifecycleManager->NotifyShutdownInConnection();
                _lifecycleManager->GetService()->SetFinalStatePromise();
            }
            else
            {
                Logging::Warn(_lifecycleManager->GetLogger(),
                              "lifecycle failed to shut down correctly - original shutdown reason was '{}'.",
                              std::move(reason));
            }
    });
}

void ShutdownState::AbortSimulation(std::string /*reason*/)
{
    ResolveAbortSimulation(std::string());
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

// ------------------------------------
// AbortingState
// ------------------------------------

void AbortingState::Initialize(std::string /*reason*/)
{
    // Ignore due to possible invalid transition between InvalidWorkflowConfig->Error->Abort->Shutdown and StartLifecycle->Initialize
}

void AbortingState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetShutdownState(),
                                                &ILifecycleState::ShutdownParticipant, std::move(reason));
}

void AbortingState::AbortSimulation(std::string /*reason*/) 
{
    // NOP: Ignore AbortSimulation() in AbortingState
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

// ------------------------------------
// ErrorState
// ------------------------------------

void ErrorState::Initialize(std::string /*reason*/)
{
  // Ignore due to possible invalid transition between InvalidWorkflowConfig->Error->Abort->Shutdown and StartLifecycle->Initialize
}

void ErrorState::RestartParticipant(std::string /*reason*/)
{
    throw SilKitError("Restart feature is currently not supported.");
}

void ErrorState::StopSimulation(std::string reason)
{
    _lifecycleManager->Shutdown(std::move(reason));
}

void ErrorState::ShutdownParticipant(std::string reason)
{
    _lifecycleManager->SetStateAndForwardIntent(_lifecycleManager->GetShuttingDownState(), &ILifecycleState::ShutdownParticipant,
                                         std::move(reason));
}

void ErrorState::AbortSimulation(std::string reason)
{
    ResolveAbortSimulation(std::move(reason));
}

void ErrorState::ResolveAbortSimulation(std::string reason)
{
    ProcessAbortCommand(std::move(reason));
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
