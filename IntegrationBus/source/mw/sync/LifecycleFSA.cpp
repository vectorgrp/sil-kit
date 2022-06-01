#include "LifecycleFSA.hpp"

namespace ib {
namespace mw {
namespace sync {

// LifecycleManagement
LifecycleManagement::LifecycleManagement(logging::ILogger* logger, LifecycleService* parentService)
    : _parentService(parentService)
    , _logger(logger)
{
    _invalidState = std::make_shared<InvalidState>(this);
    _initializedState = std::make_shared<InitializedState>(this);
    _runningState = std::make_shared<RunningState>(this);
    _pausedState = std::make_shared<PausedState>(this);
    _stoppingState = std::make_shared<StoppingState>(this);
    _stoppedState = std::make_shared<StoppedState>(this);
    _reinitializingState = std::make_shared<ReinitializingState>(this);
    _shuttingDownState = std::make_shared<ShuttingDownState>(this);
    _shutDownState = std::make_shared<ShutdownState>(this);
    _errorState = std::make_shared<ErrorState>(this);

    _currentState = _invalidState.get();
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

State* LifecycleManagement::GetInvalidState()
{
    return _invalidState.get();
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

void State::StopNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::StopHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ReinitializeNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ReinitializeHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::ShutdownHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void State::AbortSimulation(std::string reason)
{
    _context->SetState(_context->GetStoppedState(), reason);
    _context->Shutdown(std::move(reason)); // Separate "Aborted" State?
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

// InvalidState
std::string InvalidState::toString()
{
    return "Invalid";
}

auto InvalidState::GetParticipantState() -> ParticipantState
{
    return ParticipantState::Invalid;
}

// InitializedState
void InitializedState::RunSimulation(std::string reason)
{
    _context->SetState(_context->GetRunningState(), std::move(reason));
}

void InitializedState::ReinitializeHandleDone(std::string reason)
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

void RunningState::StopNotifyUser(std::string reason)
{
    _context->SetState(_context->GetStoppingState(), reason);
    // Context will set next state
    _context->HandleStop(std::move(reason));
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
    _context->SetState(_context->GetRunningState(), std::move(reason));
}

void PausedState::StopNotifyUser(std::string reason)
{
    _context->SetState(_context->GetStoppingState(), std::move(reason));
    // Context will set next state
    _context->HandleStop(std::move(reason));
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

void StoppingState::StopHandleDone(std::string reason)
{
    _context->SetState(_context->GetStoppedState(), std::move(reason));
    if (_abortRequested)
    {
        _abortRequested = false;
        _context->Shutdown("Received SystemCommand::AbortSimulation during callback.");
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

void StoppedState::StopHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void StoppedState::ReinitializeNotifyUser(std::string reason)
{
    _context->SetState(_context->GetReinitializingState(), reason);
    // Context will set next state
    _context->HandleReinitialize(std::move(reason));
}

void StoppedState::ShutdownNotifyUser(std::string reason)
{
    _context->SetState(_context->GetShuttingDownState(), reason);
    _context->HandleShutdown(std::move(reason));
}

void StoppedState::AbortSimulation(std::string reason)
{
    _context->Shutdown(std::move(reason)); // Separate "Aborted" State?
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
void ReinitializingState::ReinitializeNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ReinitializingState::ReinitializeHandleDone(std::string reason)
{
    if (_abortRequested)
    {
        _context->SetState(_context->GetStoppedState(), std::move(reason));
        _abortRequested = false;
        _context->Shutdown("Received SystemCommand::AbortSimulation during callback.");
    }
    else
    {
        _context->SetState(_context->GetInitializedState(), std::move(reason));
    }
}

void ReinitializingState::AbortSimulation(std::string reason)
{
    _abortRequested = true;
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
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
void ShuttingDownState::ShutdownNotifyUser(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ShuttingDownState::ShutdownHandleDone(std::string reason)
{
    _context->SetState(_context->GetShutdownState(), std::move(reason));
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

void ShutdownState::ShutdownHandleDone(std::string reason)
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
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::PauseSimulation(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::ContinueSimulation(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::StopNotifyUser(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::StopHandleDone(std::string reason)
{
    // TODO think about error recovery
    InvalidStateTransition(__FUNCTION__, false, std::move(reason));
}

void ErrorState::ReinitializeNotifyUser(std::string reason)
{
    // TODO think about error recovery
    _context->SetState(_context->GetReinitializingState(), std::move(reason));
    _context->HandleReinitialize(std::move(reason));
}

void ErrorState::ReinitializeHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::ShutdownNotifyUser(std::string reason)
{
    _context->SetState(_context->GetShuttingDownState(), std::move(reason));
    _context->HandleShutdown(std::move(reason));
}

void ErrorState::ShutdownHandleDone(std::string reason)
{
    InvalidStateTransition(__FUNCTION__, true, std::move(reason));
}

void ErrorState::AbortSimulation(std::string reason)
{
    _context->Shutdown(std::move(reason));
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
