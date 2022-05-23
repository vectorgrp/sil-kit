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

    virtual void RunSimulation(std::string reason);
    virtual void PauseSimulation(std::string reason);
    virtual void ContinueSimulation(std::string reason);
    virtual void StopSimulation(std::string reason);
    virtual void StopHandled(std::string reason);
    virtual void ReinitializeSimulation(std::string reason);
    virtual void ReinitializeHandled(std::string reason);
    virtual void ShutdownSimulation(std::string reason);
    virtual void ShutdownHandled(std::string reason);
    
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
    virtual void Run(std::string reason) = 0;
    virtual void Pause(std::string reason) = 0;
    virtual void Continue(std::string reason) = 0;
    virtual void Stop(std::string reason) = 0;
    virtual void StopHandled(std::string reason) = 0;
    virtual void Shutdown(std::string reason) = 0;
    virtual void ShutdownHandled(std::string reason) = 0;
    virtual void Reinitialize(std::string reason) = 0;
    virtual void ReinitializeHandled(std::string reason) = 0;
    virtual void Error(std::string reason) = 0;
    virtual void AbortSimulation(std::string reason) = 0;
};

class LifecycleManagement 
    : public ILifecycleManagement
{
public:
    LifecycleManagement(logging::ILogger* logger, LifecycleService* parentService);

    void InitLifecycleManagement(std::string reason) override 
    { 
        _currentState = GetInitializedState(); 
        _parentService->ChangeState(_currentState->GetParticipantState(), std::move(reason));
    }
    void Run(std::string reason) override { _currentState->RunSimulation(std::move(reason)); }
    void Pause(std::string reason) override { _currentState->PauseSimulation(std::move(reason)); }
    void Continue(std::string reason) override { _currentState->ContinueSimulation(std::move(reason)); }
    void Stop(std::string reason) override { _currentState->StopSimulation(std::move(reason)); }
    void StopHandled(std::string reason) override { _currentState->StopHandled(std::move(reason)); }
    void Shutdown(std::string reason) override { _currentState->ShutdownSimulation(std::move(reason)); }
    void ShutdownHandled(std::string reason) override { _currentState->ShutdownHandled(std::move(reason)); }
    void Reinitialize(std::string reason) override { _currentState->ReinitializeSimulation(std::move(reason)); }
    void ReinitializeHandled(std::string reason) override { _currentState->ReinitializeHandled(std::move(reason)); }
    void Error(std::string reason) override { _currentState->Error(std::move(reason)); }
    void AbortSimulation(std::string reason) override { _currentState->AbortSimulation(std::move(reason)); }

    // Internal state handling
    void SetState(State* state, std::string message);
    void SetStateError(std::string reason);

    State* GetOperationalState();
    State* GetErrorState();

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
    std::shared_ptr<State> _operationalState;
    std::shared_ptr<State> _errorState;

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
    //void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    void StopSimulation(std::string reason) override;
    void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
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
    //void StopSimulation(std::string reason) override;
    void StopHandled(std::string reason) override;
    void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    //void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    void ReinitializeSimulation(std::string reason) override;
    void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    //void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
    //void Error(State* lastState) override; // use default

    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
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
    //void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    void ShutdownSimulation(std::string reason) override;
    void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    //void StopSimulation(std::string reason) override;
    //void StopHandled(std::string reason) override;
    //void ReinitializeSimulation(std::string reason) override;
    //void ReinitializeHandled(std::string reason) override;
    //void ShutdownSimulation(std::string reason) override;
    void ShutdownHandled(std::string reason) override;
    //void AbortSimulation() override; // use default
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
    void StopSimulation(std::string reason) override;
    void StopHandled(std::string reason) override;
    void ReinitializeSimulation(std::string reason) override;
    void ReinitializeHandled(std::string reason) override;
    void ShutdownSimulation(std::string reason) override;
    void ShutdownHandled(std::string reason) override;
    void AbortSimulation(std::string reason) override; 
    void Error(std::string reason) override; 
    
    auto toString() -> std::string override;
    auto GetParticipantState() -> ParticipantState override;
};

} // namespace sync
} // namespace mw
} // namespace ib
