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

#include <string>

#include "LifecycleManagement.hpp"
#include "LifecycleService.hpp"
#include "LifecycleStates.hpp"
#include "TimeSyncService.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

LifecycleManagement::LifecycleManagement(Core::IParticipantInternal* participant, Services::Logging::ILogger* logger,
                                         LifecycleService* parentService)
    : _participant{participant}
    , _lifecycleService(parentService)
    , _logger(logger)
{
    _invalidState = std::make_shared<InvalidState>(this);
    _servicesCreatedState = std::make_shared<ServicesCreatedState>(this);
    _communicationInitializingState = std::make_shared<CommunicationInitializingState>(this);
    _communicationInitializedState = std::make_shared<CommunicationInitializedState>(this);
    _readyToRunState = std::make_shared<ReadyToRunState>(this);
    _runningState = std::make_shared<RunningState>(this);
    _pausedState = std::make_shared<PausedState>(this);
    _stoppingState = std::make_shared<StoppingState>(this);
    _stoppedState = std::make_shared<StoppedState>(this);
    _shuttingDownState = std::make_shared<ShuttingDownState>(this);
    _shutDownState = std::make_shared<ShutdownState>(this);
    _errorState = std::make_shared<ErrorState>(this);
    _abortingState = std::make_shared<AbortingState>(this);

    _currentState = _invalidState.get();
}

// ILifecycleManagement
void LifecycleManagement::Initialize(std::string reason)
{
    _currentState->Initialize(std::move(reason));
}

void LifecycleManagement::ServicesCreated(std::string reason)
{
    _currentState->ServicesCreated(std::move(reason));
}

void LifecycleManagement::CommunicationInitialized(std::string reason)
{
    _currentState->CommunicationInitialized(std::move(reason));
}

void LifecycleManagement::CompleteCommunicationReadyHandler(std::string reason)
{
    _currentState->CompleteCommunicationReadyHandler(std::move(reason));
}

void LifecycleManagement::ReadyToRun(std::string reason)
{
    _currentState->ReadyToRun(std::move(reason));
}

void LifecycleManagement::Restart(std::string reason)
{
    _currentState->RestartParticipant(std::move(reason));
}

void LifecycleManagement::Shutdown(std::string reason) 
{
    _currentState->ShutdownParticipant(std::move(reason));
}

void LifecycleManagement::NotifyShutdownInConnection()
{
    _participant->NotifyShutdown();
}

void LifecycleManagement::Pause(std::string reason)
{
    _currentState->PauseSimulation(std::move(reason));
}

void LifecycleManagement::Continue(std::string reason)
{
    _currentState->ContinueSimulation(std::move(reason));
}

void LifecycleManagement::Stop(std::string reason)
{
    _currentState->StopSimulation(std::move(reason));
}

void LifecycleManagement::ResolveAbortSimulation(std::string reason)
{
    _currentState->ResolveAbortSimulation(reason);
    ShutdownAfterAbort(std::move(reason));
}

void LifecycleManagement::RestartAfterStop(std::string reason)
{
    // for now, the participant will always shut down after stopping
    _lifecycleService->Restart(std::move(reason));
}

void LifecycleManagement::ShutdownAfterAbort(std::string reason)
{
    // for now, the participant will always shut down after stopping
    Shutdown(std::move(reason));
}

void LifecycleManagement::StartAutonomous(std::string reason)
{
    _currentState->ServicesCreated(reason);
}

void LifecycleManagement::Error(std::string reason)
{
    _currentState->Error(std::move(reason));
}

void LifecycleManagement::AbortSimulation(std::string reason)
{
    _currentState->AbortSimulation(std::move(reason));
    if (_currentState == GetErrorState())
    {
        GetLogger()->Warn("AbortSimulation caused a transition to an error state");
    }
}

void LifecycleManagement::CommunicationInitializing(std::string reason)
{
    _currentState->CommunicationInitializing(std::move(reason));
}

// Callback handling
CallbackResult LifecycleManagement::HandleCommunicationReady()
{
    try
    {
        auto handlerDone = _lifecycleService->TriggerCommunicationReadyHandler();
        if (handlerDone)
        {
            return CallbackResult::Completed;
        }
        else
        {
            return CallbackResult::Deferred;
        }
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return CallbackResult::Error;
    }
}

bool LifecycleManagement::HandleStarting()
{
    try
    {
        _lifecycleService->TriggerStartingHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

bool LifecycleManagement::HandleStop()
{
    try
    {
        _lifecycleService->TriggerStopHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

bool LifecycleManagement::HandleShutdown()
{
    try
    {
        _lifecycleService->TriggerShutdownHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

bool LifecycleManagement::HandleAbort()
{
    if (!_lastBeforeAbortingState)
    {
        throw SilKit::StateError("Abort handler was about to be triggered without knowing which state was active "
                                 "before abort was called.");
    }

    try
    {
        _lifecycleService->TriggerAbortHandler(_lastBeforeAbortingState->GetParticipantState());
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}


void LifecycleManagement::StartTime()
{
    (dynamic_cast<TimeSyncService*>(_lifecycleService->GetTimeSyncService()))->StartTime();
}

void LifecycleManagement::AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    _lifecycleService->AddAsyncSubscriptionsCompletionHandler(std::move(handler));
}

void LifecycleManagement::SetState(ILifecycleState* newState, std::string reason)
{
    UpdateLifecycleState(newState);
    UpdateParticipantState(std::move(reason));
}

void LifecycleManagement::SetStateAndForwardIntent(ILifecycleState* newState,
                                                 void (ILifecycleState::*intent)(std::string), std::string reason)
{
    UpdateLifecycleState(newState);
    // NB: UpdateParticipantState can alter _currentState if the ParticipantState change causes a SystemState change.
    // This addressed by NOPs in the new state for the original intent.
    UpdateParticipantState(reason);
    (_currentState->*intent)(std::move(reason));
}

void LifecycleManagement::UpdateLifecycleState(ILifecycleState* newState)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};

    if (newState == GetAbortingState())
    {
        _lastBeforeAbortingState = _currentState;
    }
    _currentState = newState;
}

void LifecycleManagement::UpdateParticipantState(std::string reason)
{
    _lifecycleService->ChangeParticipantState(_currentState->GetParticipantState(), std::move(reason));
}

ILifecycleState* LifecycleManagement::GetCurrentState()
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentState;
}

ILifecycleState* LifecycleManagement::GetInvalidState()
{
    return _invalidState.get();
}

ILifecycleState* LifecycleManagement::GetOperationalState()
{
    return _operationalState.get();
}

ILifecycleState* LifecycleManagement::GetErrorState()
{
    return _errorState.get();
}

ILifecycleState* LifecycleManagement::GetAbortingState()
{
    return _abortingState.get();
}

ILifecycleState* LifecycleManagement::GetServicesCreatedState()
{
    return _servicesCreatedState.get();
}

ILifecycleState* LifecycleManagement::GetCommunicationInitializingState()
{
    return _communicationInitializingState.get();
}

ILifecycleState* LifecycleManagement::GetCommunicationInitializedState()
{
    return _communicationInitializedState.get();
}

ILifecycleState* LifecycleManagement::GetReadyToRunState()
{
    return _readyToRunState.get();
}

ILifecycleState* LifecycleManagement::GetRunningState()
{
    return _runningState.get();
}

ILifecycleState* LifecycleManagement::GetPausedState()
{
    return _pausedState.get();
}

ILifecycleState* LifecycleManagement::GetStoppingState()
{
    return _stoppingState.get();
}

ILifecycleState* LifecycleManagement::GetStoppedState()
{
    return _stoppedState.get();
}

ILifecycleState* LifecycleManagement::GetShuttingDownState()
{
    return _shuttingDownState.get();
}

ILifecycleState* LifecycleManagement::GetShutdownState()
{
    return _shutDownState.get();
}

Services::Logging::ILogger* LifecycleManagement::GetLogger()
{
    return _logger;
}

LifecycleService* LifecycleManagement::GetService()
{
    return _lifecycleService;
}

OperationMode LifecycleManagement::GetOperationMode() const
{
    return _lifecycleService->GetOperationMode();
}

Core::IParticipantInternal* LifecycleManagement::GetParticipant()
{
    return _participant;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
