#include <memory>

#include "ib/mw/logging/ILogger.hpp"

#include "LifecycleService.hpp"
#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class LifecycleManagement;

class State
{
public:
    State(LifecycleManagement* context)
    : _context(context)
    {}

public:
    virtual ~State() = default;

    virtual void EstablishCommunication(std::string reason);
    virtual void CommunicationReadyNotifyUser(std::string reason);
    virtual void CommunicationReadyHandleDone(std::string reason);

    virtual void RunSimulation(std::string reason);
    virtual void PauseSimulation(std::string reason);
    virtual void ContinueSimulation(std::string reason);
    virtual void StopNotifyUser(std::string reason);
    virtual void StopHandleDone(std::string reason);
    virtual void ReinitializeNotifyUser(std::string reason);
    virtual void ReinitializeHandleDone(std::string reason);
    virtual void ShutdownNotifyUser(std::string reason);
    virtual void ShutdownHandleDone(std::string reason);
    
    virtual void AbortSimulation(std::string reason);
    virtual void Error(std::string reason);

    virtual void NewSystemState(SystemState systemState);

    virtual auto toString() -> std::string = 0;
    virtual auto GetParticipantState() -> ParticipantState = 0;

protected:
    void InvalidStateTransition(std::string transitionName, bool triggerErrorState, std::string originalReason);

protected:
    LifecycleManagement* _context;
};

class ILifecycleManagement
{
public:
    virtual ~ILifecycleManagement() = default;

public:
    virtual void InitLifecycleManagement(std::string reason) = 0;
    virtual void AllControllersAvailable(std::string reason) = 0;
    virtual void AllCommunicationReady(std::string reason) = 0;
    virtual void Run(std::string reason) = 0;
    virtual void Pause(std::string reason) = 0;
    virtual void Continue(std::string reason) = 0;
    virtual void Stop(std::string reason) = 0;
    virtual bool Shutdown(std::string reason) = 0; // shutdown successful?
    virtual void Reinitialize(std::string reason) = 0;
    virtual void Error(std::string reason) = 0;
    virtual bool AbortSimulation(std::string reason) = 0; // Abort->Shutdown successful?
};

class LifecycleManagement 
    : public ILifecycleManagement
{
public:
    LifecycleManagement(logging::ILogger* logger, LifecycleService* parentService);

    void InitLifecycleManagement(std::string reason) override 
    { 
        _currentState = GetControllersCreatedState(); 
        _parentService->ChangeState(_currentState->GetParticipantState(), std::move(reason));
    }
    void AllControllersAvailable(std::string reason) { _currentState->EstablishCommunication(std::move(reason)); }
    void AllCommunicationReady(std::string reason) 
    { 
        _currentState->CommunicationReadyNotifyUser(std::move(reason));
        _currentState->CommunicationReadyHandleDone("Finished CommunicationReady execution.");
    }
    void Run(std::string reason) override { _currentState->RunSimulation(std::move(reason)); }
    void Pause(std::string reason) override { _currentState->PauseSimulation(std::move(reason)); }
    void Continue(std::string reason) override { _currentState->ContinueSimulation(std::move(reason)); }
    void Stop(std::string reason) override 
    { 
        _currentState->StopNotifyUser(reason);
        // Could be Stopping or Error state
        _currentState->StopHandleDone("Finished StopHandle execution.");
    }
    bool Shutdown(std::string reason) override 
    {
        _currentState->ShutdownNotifyUser(reason);
        return (_currentState != GetErrorState());
    }
    void Reinitialize(std::string reason) override
    {
        _currentState->ReinitializeNotifyUser(reason);
        _currentState->ReinitializeHandleDone(std::move(reason));
    }
    void Error(std::string reason) override { _currentState->Error(std::move(reason)); }
    bool AbortSimulation(std::string reason) override 
    { 
        _currentState->AbortSimulation(std::move(reason));
        return (_currentState != GetErrorState());
    }

public:
    void HandleCommunicationReady(std::string reason)
    {
        try
        {
            _parentService->TriggerCommunicationReadyHandle(std::move(reason));
        }
        catch (const std::exception&)
        {
            // Switch to error state if handle triggers error
            SetStateError("Exception during CommunicationReadyHandle execution.");
        }
    }

    void HandleReinitialize(std::string reason)
    {
        try
        {
            _parentService->TriggerReinitializeHandle(std::move(reason));
        }
        catch (const std::exception&)
        {
            // Switch to error state if handle triggers error
            SetStateError("Exception during StopHandle execution.");
        }
    }

    void HandleStop(std::string reason)
    {
        try
        {
            _parentService->TriggerStopHandle(std::move(reason));
        }
        catch (const std::exception&)
        {
            // Switch to error state if handle triggers error
            SetStateError("Exception during StopHandle execution.");
        }
    }

    void HandleShutdown(std::string reason) 
    {
        try
        {
            _parentService->TriggerShutdownHandle(std::move(reason));
            _currentState->ShutdownHandleDone("Finished ShutdownHandle execution.");
        }
        catch (const std::exception&)
        {
            _currentState->ShutdownHandleDone("Exception during ShutdownHandle execution - shutting down anyway.");
        }
    }

    // Internal state handling
    void SetState(State* state, std::string message);
    void SetStateError(std::string reason);

    State* GetInvalidState();
    State* GetOperationalState();
    State* GetErrorState();

    State* GetControllersCreatedState();
    State* GetCommunicationReadyState();
    State* GetInitializedState();
    State* GetRunningState();
    State* GetPausedState();
    State* GetStoppingState();
    State* GetStoppedState();
    State* GetReinitializingState();
    State* GetShuttingDownState();
    State* GetShutdownState();

    logging::ILogger* GetLogger();

private:
    std::shared_ptr<State> _invalidState;
    std::shared_ptr<State> _operationalState;
    std::shared_ptr<State> _errorState;

    std::shared_ptr<State> _controllersCreatedState;
    std::shared_ptr<State> _communicationReadyState;
    std::shared_ptr<State> _initializedState;
    std::shared_ptr<State> _runningState;
    std::shared_ptr<State> _pausedState;
    std::shared_ptr<State> _stoppingState;
    std::shared_ptr<State> _stoppedState;
    std::shared_ptr<State> _reinitializingState;
    std::shared_ptr<State> _shuttingDownState;
    std::shared_ptr<State> _shutDownState;

    State* _currentState;
    LifecycleService* _parentService;

    logging::ILogger* _logger;
};

class InvalidState
    : public State
{
public:
    InvalidState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandleDone(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ControllersCreatedState
    : public State
{
public:
    ControllersCreatedState(LifecycleManagement* context)
        : State(context)
    {
    }
    
    void EstablishCommunication(std::string reason) override;
    //void CommunicationReadyNotifyUser(std::string reason) override;
    //void CommunicationReadyHandleDone(std::string reason) override;
    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandleDone(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class CommunicationReadyState 
    : public State
{
public:
    CommunicationReadyState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void EstablishCommunication(std::string reason) override;
    void CommunicationReadyNotifyUser(std::string reason) override;
    void CommunicationReadyHandleDone(std::string reason) override;
    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandleDone(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class InitializedState
    : public State
{
public:
    InitializedState(LifecycleManagement* context)
        : State(context)
    {
    }

    void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    void ReinitializeHandleDone(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class RunningState 
    : public State
{
public:
    RunningState(LifecycleManagement* context)
        : State(context)
    {
    }

    void RunSimulation(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};


class PausedState 
    : public State
{
public:
    PausedState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    //void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class StoppingState 
    : public State
{
public:
    StoppingState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    void StopHandleDone(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    bool _abortRequested = false;
};

class StoppedState 
    : public State
{
public:
    StoppedState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    void StopHandleDone(std::string reason) override;
    void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override;
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ReinitializingState 
    : public State
{
public:
    ReinitializingState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    void ReinitializeNotifyUser(std::string reason) override;
    void ReinitializeHandleDone(std::string reason) override;
    //void ShutdownNotifyUser(std::string reason) override;
    //void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;

private:
    bool _abortRequested = false;
};

class ShuttingDownState 
    : public State
{
public:
    ShuttingDownState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override;
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ShutdownState 
    : public State
{
public:
    ShutdownState(LifecycleManagement* context)
        : State(context)
    {
    }

    //void RunSimulation(std::string reason) override;
    //void PauseSimulation(std::string reason) override;
    //void ContinueSimulation(std::string reason) override;
    //void StopNotifyUser(std::string reason) override;
    //void HandleStop(std::string reason) override;
    //void ReinitializeNotifyUser(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override;
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

class ErrorState 
    : public State
{
public:
    ErrorState(LifecycleManagement* context)
        : State(context)
    {
    }

    void RunSimulation(std::string reason) override;
    void PauseSimulation(std::string reason) override;
    void ContinueSimulation(std::string reason) override;
    void StopNotifyUser(std::string reason) override;
    void StopHandleDone(std::string reason) override;
    void ReinitializeNotifyUser(std::string reason) override;
    void ReinitializeHandleDone(std::string reason) override;
    void ShutdownNotifyUser(std::string reason) override;
    void ShutdownHandleDone(std::string reason) override;
    void AbortSimulation(std::string reason) override; 
    void Error(std::string reason) override; 
    
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

} // namespace sync
} // namespace mw
} // namespace ib
