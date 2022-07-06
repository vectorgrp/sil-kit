// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <string>

#include "LifecycleManagement.hpp"
#include "LifecycleStates.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

// LifecycleManagement
LifecycleManagement::LifecycleManagement(Services::Logging::ILogger* logger, LifecycleService* parentService)
    : _parentService(parentService)
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

    _currentState = _invalidState.get();
}

void LifecycleManagement::InitLifecycleManagement(std::string reason)
{
    _currentState = GetServicesCreatedState();
    _parentService->ChangeState(GetServicesCreatedState()->GetParticipantState(), std::move(reason));
}

void LifecycleManagement::SkipSetupPhase(std::string reason)
{
    _currentState->NewSystemState(SystemState::ServicesCreated);
    _currentState->NewSystemState(SystemState::CommunicationInitializing);
    _currentState->NewSystemState(SystemState::CommunicationInitialized);
    _currentState->NewSystemState(SystemState::ReadyToRun);
    _currentState->RunSimulation(std::move(reason));
}

void LifecycleManagement::NewSystemState(SystemState systemState)
{
    _currentState->NewSystemState(systemState);
}

void LifecycleManagement::Run(std::string reason)
{
    _currentState->RunSimulation(std::move(reason));
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
    _currentState->StopNotifyUser(reason);
    // Could be Stopping or Error state
    _currentState->StopHandlerDone("Finished StopHandle execution.");
}

bool LifecycleManagement::Shutdown(std::string reason)
{
    _currentState->ShutdownNotifyUser(reason);
    return (_currentState != GetErrorState());
}

void LifecycleManagement::Restart(std::string reason)
{
    _currentState->Restart(std::move(reason));
}

void LifecycleManagement::Error(std::string reason)
{
    _currentState->Error(std::move(reason));
}

bool LifecycleManagement::AbortSimulation(std::string reason)
{
    _currentState->AbortSimulation(std::move(reason));
    return (_currentState != GetErrorState());
}

void LifecycleManagement::HandleCommunicationReady(std::string reason)
{
    try
    {
        _parentService->TriggerCommunicationReadyHandler(std::move(reason));
    }
    catch (const std::exception&)
    {
        // Switch to error state if handle triggers error
        SetStateError("Exception during CommunicationReadyHandle execution.");
        return;
    }
    SetState(GetReadyToRunState(), std::move(reason));
}

void LifecycleManagement::HandleStarting(std::string reason)
{
    try
    {
        _parentService->TriggerStartingHandler(
            "Transition to ParticipantState::Running imminent and virtual timeSync inactive.");
    }
    catch (const std::exception&)
    {
        // Switch to error state if handle triggers error
        SetStateError("Exception during CommunicationReadyHandle execution.");
        return;
    }
    SetState(GetRunningState(), std::move(reason));
}

void LifecycleManagement::HandleStop(std::string reason)
{
    try
    {
        _parentService->TriggerStopHandler(std::move(reason));
    }
    catch (const std::exception&)
    {
        // Switch to error state if handle triggers error
        SetStateError("Exception during StopHandle execution.");
    }
}

void LifecycleManagement::HandleShutdown(std::string reason)
{
    try
    {
        _parentService->TriggerShutdownHandler(std::move(reason));
        _currentState->ShutdownHandlerDone("Finished ShutdownHandle execution.");
    }
    catch (const std::exception&)
    {
        _currentState->ShutdownHandlerDone("Exception during ShutdownHandle execution - shutting down anyway.");
    }
}

void LifecycleManagement::SetState(ILifecycleState* state, std::string message)
{
    _currentState = state;
    _parentService->ChangeState(_currentState->GetParticipantState(), std::move(message));
}

void LifecycleManagement::SetStateError(std::string reason)
{
    SetState(GetErrorState(), reason);
    _currentState->Error(std::move(reason));
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
    return _parentService;
}

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
