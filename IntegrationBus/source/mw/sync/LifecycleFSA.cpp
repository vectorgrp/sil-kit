#include "LifecycleFSA.hpp"

namespace ib {
namespace mw {
namespace sync {

// LifecycleManagement
LifecycleManagement::LifecycleManagement(logging::ILogger* logger, LifecycleService* parentService)
    : _parentService(parentService)
    , _logger(logger)
{
    _initializedState = std::make_shared<InitializedState>(this);
    _runningState = std::make_shared<RunningState>(this);
    _pausedState = std::make_shared<PausedState>(this);
    _stoppingState = std::make_shared<StoppingState>(this);
    _stoppedState = std::make_shared<StoppedState>(this);
    _reinitializingState = std::make_shared<ReinitializingState>(this);
    _shuttingDownState = std::make_shared<ShuttingDownState>(this);
    _shutDownState = std::make_shared<ShutdownState>(this);
    _errorState = std::make_shared<ErrorState>(this);
}

void LifecycleManagement::SetState(State* state, std::string message)
{
    _currentState = state;
    _parentService->ChangeState(_currentState->GetParticipantState(), std::move(message));
}

void LifecycleManagement::SetStateError(std::string reason)
{
    SetState(GetErrorState(), reason);
    _currentState->Error(std::move(reason));
}

State* LifecycleManagement::GetOperationalState()
{
    return _operationalState.get();
}

State* LifecycleManagement::GetErrorState()
{
    return _errorState.get();
}

State* LifecycleManagement::GetInitializedState()
{
    return _initializedState.get();
}

State* LifecycleManagement::GetRunningState()
{
    return _runningState.get();
}

State* LifecycleManagement::GetPausedState()
{
    return _pausedState.get();
}

State* LifecycleManagement::GetStoppingState()
{
    return _stoppingState.get();
}

State* LifecycleManagement::GetStoppedState()
{
    return _stoppedState.get();
}

State* LifecycleManagement::GetReinitializingState()
{
    return _reinitializingState.get();
}

State* LifecycleManagement::GetShuttingDownState()
{
    return _shuttingDownState.get();
}

State* LifecycleManagement::GetShutdownState()
{
    return _shutDownState.get();
}

logging::ILogger* LifecycleManagement::GetLogger()
{
    return _logger;
}

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

void State::StopSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::StopHandled(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ReinitializeSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ReinitializeHandled(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownHandled(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::AbortSimulation(std::string reason)
{
    _context->SetState(_context->GetShuttingDownState(), std::move(reason));
    _context->Shutdown(std::move(reason));
}

void State::Error(std::string reason)
{
    _context->SetStateError(std::move(reason));
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
        _context->Error(ss.str());
    }
    else
    {
        _context->GetLogger()->Warn(ss.str());
    }
}

// InitializedState
void InitializedState::RunSimulation(std::string reason)
{
    _context->SetState(_context->GetRunningState(), std::move(reason));
}

void InitializedState::ReinitializeHandled(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

std::string InitializedState::toString()
{
    return "Initialized";
}

auto InitializedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Initialized;
}

// RunningState
void RunningState::RunSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void RunningState::PauseSimulation(std::string reason)
{
    _context->SetState(_context->GetPausedState(), std::move(reason));
}

void RunningState::ContinueSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void RunningState::StopSimulation(std::string reason)
{
    _context->SetState(_context->GetStoppingState(), std::move(reason));
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
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void PausedState::ContinueSimulation(std::string reason)
{
    _context->SetState(_context->GetRunningState(), std::move(reason));
}

void PausedState::StopSimulation(std::string reason)
{
    _context->SetState(_context->GetStoppingState(), std::move(reason));
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
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppingState::StopHandled(std::string reason)
{
    _context->SetState(_context->GetStoppedState(), std::move(reason));
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
void StoppedState::StopHandled(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppedState::ReinitializeSimulation(std::string reason)
{
    _context->SetState(_context->GetReinitializingState(), std::move(reason));
}

void StoppedState::ShutdownSimulation(std::string reason)
{
    _context->SetState(_context->GetShuttingDownState(), std::move(reason));
}

auto StoppedState::toString() -> std::string
{
    return "Stopped";
}

auto StoppedState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Stopped;
}

// ReinitializingState
void ReinitializingState::ReinitializeSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ReinitializingState::ReinitializeHandled(std::string reason)
{
    _context->SetState(_context->GetInitializedState(), std::move(reason));
}

auto ReinitializingState::toString() -> std::string
{
    return "Reinitializing";
}

auto ReinitializingState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Reinitializing;
}

// ShuttingDownState
void ShuttingDownState::ShutdownSimulation(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ShuttingDownState::ShutdownHandled(std::string reason)
{
    _context->SetState(_context->GetShutdownState(), std::move(reason));
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
void ShutdownState::ShutdownHandled(std::string reason)
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
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::PauseSimulation(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::ContinueSimulation(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::StopSimulation(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::StopHandled(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::ReinitializeSimulation(std::string reason)
{
    // TODO think about error recovery
    _context->SetState(_context->GetReinitializingState(), std::move(reason));
}

void ErrorState::ReinitializeHandled(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::ShutdownSimulation(std::string reason)
{
    // TODO think about error recovery
    _context->SetState(_context->GetShutdownState(), std::move(reason));
}

void ErrorState::ShutdownHandled(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::AbortSimulation(std::string reason)
{
    // TODO think about error recovery
    _context->SetState(_context->GetShutdownState(), std::move(reason));
}

void ErrorState::Error(std::string reason)
{
    _context->GetLogger()->Warn("Received error transition within error state. Original reason: " + std::move(reason));
}

auto ErrorState::toString() -> std::string
{
    return "Error";
}

auto ErrorState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Error;
}

} // namespace sync
} // namespace mw
} // namespace ib
