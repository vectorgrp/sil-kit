#include "ParticipantController.hpp"

#include <cassert>

#include "ib/cfg/string_utils.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

// anonymous namespace for local helper functions
namespace {

struct SyncPolicyTimeQuantum
{
    inline static void RequestStep(const ParticipantController& controller)
    {
        controller.SendQuantumRequest();
    }
    inline static void FinishedStep(ParticipantController& controller)
    {
        controller.AdvanceQuantum();
    }
};

struct SyncPolicyDiscreteTime
{
    inline static void RequestStep(const ParticipantController& /*controller*/) {}
    inline static void FinishedStep(const ParticipantController& controller)
    {
        controller.SendTickDone();
    }
};

struct SyncPolicyDiscreteTimePassive
{
    inline static void RequestStep(const ParticipantController& /*controller*/) {}
    inline static void FinishedStep(const ParticipantController& /*controller*/) {}
};


template <class SyncPolicy>
class TaskRunnerAsync
    : public ITaskRunner
    , public SyncPolicy
{
public:
    TaskRunnerAsync(ParticipantController& controller)
        : _controller{controller}
    {
    }
    void Start() override {}
    void Initialize() override {}
    void Run() override
    {
        SyncPolicy::RequestStep(_controller);
    }
    void GrantReceived() override
    {
        _controller.ExecuteSimTask();
        SyncPolicy::FinishedStep(_controller);
        SyncPolicy::RequestStep(_controller);
    }
    void Stop() override {}
    void Shutdown() override {}

private:
    ParticipantController& _controller;
};

template <class SyncPolicy>
class TaskRunner
    : public ITaskRunner
    , public SyncPolicy
{
public:
    TaskRunner(ParticipantController& controller)
        : _controller{controller}
    {
    }
    void Start() override
    {
        while (WaitForRunCommand())
        {
            while (true)
            {
                SyncPolicy::RequestStep(_controller);

                if (!GetGrant())
                    break;

                _controller.ExecuteSimTask();
                SyncPolicy::FinishedStep(_controller);
            }
        }
    }
    void Initialize() override
    {
    }
    void Run() override
    {
        SetRunPromise(true);
    }
    void GrantReceived() override
    {
        SetGrant(true);
    }
    void Stop() override
    {
        SetGrant(false);
    }
    void Shutdown() override
    {
        SetRunPromise(false);
    }

private:
    inline bool WaitForRunCommand()
    {
        auto future = _runPromise.get_future();
        auto running = future.get();

        // Make New Promise
        std::lock_guard<std::mutex> guard{_runMutex};
        _runPromise = std::promise<bool>{};
        if (_controller.State() == ParticipantState::Shutdown)
        {
            _runPromise.set_value(false);
            return false;
        }
        return running;
    }
    inline void SetRunPromise(bool running)
    {
        std::lock_guard<std::mutex> guard{_runMutex};
        try
        {
            _runPromise.set_value(running);
        }
        catch (const std::future_error& e)
        {
            // we accept that the promise is already set by another thread or callback.
            if (e.code() != std::future_errc::promise_already_satisfied)
                throw e;
        }
    }

    inline void SetGrant(bool grant)
    {
        std::lock_guard<std::mutex> guard{_grantMutex};
        try
        {
            _grantPromise.set_value(grant);
        }
        catch (const std::future_error& e)
        {
            // we accept that the promise is already set by another thread or callback.
            if (e.code() != std::future_errc::promise_already_satisfied)
                throw e;
        }
    }
    inline bool GetGrant()
    {
        auto future = _grantPromise.get_future();
        auto grant = future.get();

        // Make New Promise
        std::lock_guard<std::mutex> guard{_grantMutex};
        _grantPromise = std::promise<bool>{};
        if (_controller.State() == ParticipantState::Stopped ||
            _controller.State() == ParticipantState::Error)
        {
            return false;
        }

        return grant;
    }

    ParticipantController& _controller;
    std::promise<bool> _runPromise;
    std::mutex _runMutex;
    std::promise<bool> _grantPromise;
    std::mutex _grantMutex;
    bool _shutdown{false};
};

} // anonymous namespace for local helper functions


ParticipantController::ParticipantController(IComAdapter* comAdapter, cfg::Participant participantConfig, cfg::TimeSync timeyncConfig)
    : _comAdapter{comAdapter}
    , _participantConfig(std::move(participantConfig))
    , _timesyncConfig(std::move(timeyncConfig))
{
    _status.participantName = _participantConfig.name;
}

void ParticipantController::SetInitHandler(InitHandlerT handler)
{
    _initHandler = std::move(handler);
}

void ParticipantController::SetStopHandler(StopHandlerT handler)
{
    _stopHandler = std::move(handler);
}

void ParticipantController::SetShutdownHandler(ShutdownHandlerT handler)
{
    _shutdownHandler = std::move(handler);
}

void ParticipantController::SetSimulationTask(SimTaskT task)
{
    _simTask = std::move(task);
}

void ParticipantController::SetPeriod(std::chrono::nanoseconds period)
{
    _period = period;
}

void ParticipantController::SetEarliestEventTime(std::chrono::nanoseconds eventTime)
{
    throw std::exception();
}

template <template <class> class TaskRunnerT>
void ParticipantController::StartTaskRunner()
{
    if (!_simTask)
    {
        ReportError("ParticipantController::Run() was called without having set a SimTask!");
        throw std::exception();
    }

    switch (_participantConfig.syncType)
    {
    case cfg::SyncType::DiscreteEvent:
        throw std::runtime_error("Unsupported SyncType " + to_string(_participantConfig.syncType));
    case cfg::SyncType::TimeQuantum:
        _taskRunner = std::make_unique<TaskRunnerT<SyncPolicyTimeQuantum>>(*this);
        break;
    case cfg::SyncType::DiscreteTime:
        _taskRunner = std::make_unique<TaskRunnerT<SyncPolicyDiscreteTime>>(*this);
        break;
    case cfg::SyncType::DiscreteTimePassive:
        _taskRunner = std::make_unique<TaskRunnerT<SyncPolicyDiscreteTimePassive>>(*this);
        break;
    default:
        throw ib::cfg::Misconfiguration("Invalid SyncType " + to_string(_participantConfig.syncType));
    }

    ChangeState(ParticipantState::Idle, "ParticipantController::Run() was called");
    _taskRunner->Start();
}

auto ParticipantController::Run() -> ParticipantState
{
    StartTaskRunner<TaskRunner>();
    return State();
}

auto ParticipantController::RunAsync() -> std::future<ParticipantState>
{
    if (_timesyncConfig.syncPolicy == cfg::TimeSync::SyncPolicy::Strict)
    {
        std::cerr << "ERROR: ParticipantController::RunAsync() cannot be used when SyncPolicy::Strict is configured" << std::endl;
        ChangeState(ParticipantState::Error, "ParticipantController::RunAsync() cannot be used when SyncPolicy::Strict is configured");
        _finalStatePromise.set_value(State());
        return _finalStatePromise.get_future();
    }

    StartTaskRunner<TaskRunnerAsync>();
    return _finalStatePromise.get_future();
}

void ParticipantController::ReportError(std::string errorMsg)
{
    ChangeState(ParticipantState::Error, std::move(errorMsg));
}

void ParticipantController::Pause(std::string reason)
{
    if (State() != ParticipantState::Running)
    {
        std::string errorMessage{"ParticipantController::Pause() was called in state ParticipantState::" + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    ChangeState(ParticipantState::Paused, std::move(reason));
}

void ParticipantController::Continue()
{
    if (State() != ParticipantState::Paused)
    {
        std::string errorMessage{"ParticipantController::Continue() was called in state ParticipantState::" + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    ChangeState(ParticipantState::Running, "Pause finished");
}

void ParticipantController::Stop(std::string reason)
{
    ChangeState(ParticipantState::Stopping, std::move(reason));

    if (_taskRunner)
        _taskRunner->Stop();

    if (_stopHandler)
    {
        try
        {
            _stopHandler();
            // The handler can report an error, which overrules the default transition to ParticipantState::Stopped
            if (State() != ParticipantState::Error)
            {
                reason += "; StopHandler completed successfully.";
                ChangeState(ParticipantState::Stopped, std::move(reason));
            }
        }
        catch (const std::exception& e)
        {
            reason += "; StopHandler threw exception: ";
            reason += e.what();
            ChangeState(ParticipantState::Stopped, std::move(reason));
        }
    }
    else
    {
        reason += "; no StopHandler registered.";
        ChangeState(ParticipantState::Stopped, reason);
    }

}

void ParticipantController::Shutdown(std::string reason)
{
    ChangeState(ParticipantState::ShuttingDown, reason);

    _taskRunner->Shutdown();

    if (_shutdownHandler)
    {
        try
        {
            _shutdownHandler();
            reason += "; ShutdownHandler completed.";
            ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
        catch (const std::exception& e)
        {
            reason += "; ShutdownHandler threw exception: ";
            reason += e.what();
            ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
    }
    else
    {
        reason += "; no ShutdownHandler registered.";
        ChangeState(ParticipantState::Shutdown, std::move(reason));
    }

    _finalStatePromise.set_value(State());
}


auto ParticipantController::State() const -> ParticipantState
{
    return _status.state;
}

auto ParticipantController::Status() const -> const ParticipantStatus&
{
    return _status;
}

auto ParticipantController::Now() const -> std::chrono::nanoseconds
{
    return _now;
}

void ParticipantController::SetEndpointAddress(const mw::EndpointAddress& addr)
{
    _endpointAddress = addr;
}

auto ParticipantController::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddress;
}     

void ParticipantController::ReceiveIbMessage(ib::mw::EndpointAddress /*from*/, const ParticipantCommand& command)
{
    if (command.participant != _endpointAddress.participant)
        return;

    ChangeState(ParticipantState::Initializing, "Received ParticipantCommand::" + to_string(command.kind));
    if (_initHandler)
    {
        try
        {
            _initHandler(command);
        }
        catch (const std::exception& e)
        {
            ReportError(std::string{"InitHandler did throw an exception: "} + e.what());
            return;
        }
    }

    _now = 0ns;
    _taskRunner->Initialize();
    ChangeState(ParticipantState::Initialized, "InitHandler completed without exception.");
}

void ParticipantController::ReceiveIbMessage(ib::mw::EndpointAddress from, const SystemCommand& command)
{
    if (!_taskRunner)
    {
        ReportError("Received SystemCommand::" + to_string(command.kind) + " before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (command.kind)
    {
    case SystemCommand::Kind::Run:
        if (State() == ParticipantState::Initialized)
        {
            ChangeState(ParticipantState::Running, "Received SystemCommand::Run");
            _taskRunner->Run();
            return;
        }
        break;

    case SystemCommand::Kind::Stop:
        if (State() == ParticipantState::Running)
        {
            Stop("Received SystemCommand::Stop");
            return;
        }
        break;

    case SystemCommand::Kind::Shutdown:
        if (State() == ParticipantState::Error || State() == ParticipantState::Stopped)
        {
            Shutdown("Received SystemCommand::Shutdown");
            return;
        }
        break;

    default:
        assert(false);
    }

    // We should not reach this point in normal operation.
    ReportError("Received SystemCommand::" + to_string(command.kind) + " while in ParticipantState::" + to_string(State()));
}

void ParticipantController::ReceiveIbMessage(mw::EndpointAddress from, const Tick& msg)
{
    switch (_participantConfig.syncType)
    {
    case cfg::SyncType::DiscreteTime:
    case cfg::SyncType::DiscreteTimePassive:
        break;
    default:
        return;
    }

    if (!_taskRunner)
    {
        ReportError("Received TICK before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (State())
    {
    case ParticipantState::Invalid:
        // [[fallthrough]]
    case ParticipantState::Idle:
        // [[fallthrough]]
    case ParticipantState::Initializing:
        // [[fallthrough]]
    case ParticipantState::Initialized:
        ReportError("Received TICK in state ParticipantState::" + to_string(State()));
        return;

    case ParticipantState::Paused:
        // We have to process TICKS received in Paused. This can happen due to
        // race conditions, and we can't undo a TICK. It should not occur more
        // than once though.
        // [[fallthrough]]
    case ParticipantState::Running:
        _now = msg.now;
        _taskRunner->GrantReceived();
        break;
        
    case ParticipantState::Stopping:
        return; // ignore TICK during stop/shutdown procedure
    case ParticipantState::Stopped:
        return; // ignore TICK during stop/shutdown procedure
    case ParticipantState::Error:
        return; // ignore TICK during stop/shutdown procedure
    case ParticipantState::ShuttingDown:
        return; // ignore TICK during stop/shutdown procedure
    case ParticipantState::Shutdown:
        return; // ignore TICK during stop/shutdown procedure
    default:
        ReportError("Received TICK in state ParticipantState::" + to_string(State()));
        return;
    }
}

void ParticipantController::ReceiveIbMessage(mw::EndpointAddress from, const QuantumGrant& msg)
{
    if (_participantConfig.syncType != cfg::SyncType::TimeQuantum)
        return;

    if (_endpointAddress != msg.grantee)
        return;

    if (!_taskRunner)
    {
        ReportError("Received QuantumGrant before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (State())
    {
    case ParticipantState::Invalid:
        // [[fallthrough]]
    case ParticipantState::Idle:
        // [[fallthrough]]
    case ParticipantState::Initializing:
        // [[fallthrough]]
    case ParticipantState::Initialized:
        ReportError("Received QuantumGrant in state ParticipantState::" + to_string(State()));
        return;

    case ParticipantState::Paused:
        // We have to process QuantumGrants received in Paused. This can happen
        // due to race conditions, and we can't undo a TICK. It should not occur
        // more than once though.
        // [[fallthrough]]
    case ParticipantState::Running:
        ProcessQuantumGrant(msg);
        return;

    case ParticipantState::Stopping:
        return; // ignore QuantumGrants during stop/shutdown procedure
    case ParticipantState::Stopped:
        return; // ignore QuantumGrants during stop/shutdown procedure
    case ParticipantState::Error:
        return; // ignore QuantumGrants during stop/shutdown procedure
    case ParticipantState::ShuttingDown:
        return; // ignore QuantumGrants during stop/shutdown procedure
    case ParticipantState::Shutdown:
        return; // ignore QuantumGrants during stop/shutdown procedure

    default:
        ReportError("Received QuantumGrant in state ParticipantState::" + to_string(State()));
        return;
    }

    switch (msg.status)
    {
    case QuantumRequestStatus::Granted:
        _now = msg.now;
        if (msg.duration != _period)
        {
            ReportError("Granted quantum duration does not match request!");
        }
        else
        {
            _taskRunner->GrantReceived();
        }
        break;
    case QuantumRequestStatus::Rejected:
        _now = msg.now;
        _taskRunner->Stop();
        break;
    case QuantumRequestStatus::Invalid:
        ReportError("Received invalid QuantumGrant");
        break;
    default:
        ReportError("Received QuantumGrant with unknown Status");
    }
}

void ParticipantController::SendTickDone() const
{
    _comAdapter->WaitForMessageDelivery();
    SendIbMessage(TickDone{});
}

void ParticipantController::SendQuantumRequest() const
{
    _comAdapter->WaitForMessageDelivery();
    SendIbMessage(QuantumRequest{_now, _period});
}

void ParticipantController::ProcessQuantumGrant(const QuantumGrant& msg)
{
    switch (msg.status)
    {
    case QuantumRequestStatus::Granted:
        _now = msg.now;
        if (msg.duration != _period)
        {
            ReportError("Granted quantum duration does not match request!");
        }
        else
        {
            _taskRunner->GrantReceived();
        }
        break;
    case QuantumRequestStatus::Rejected:
        _now = msg.now;
        _taskRunner->Stop();
        break;
    case QuantumRequestStatus::Invalid:
        ReportError("Received invalid QuantumGrant");
        break;
    default:
        ReportError("Received QuantumGrant with unknown Status");
    }
}

void ParticipantController::AdvanceQuantum()
{
    _now += _period;
}


void ParticipantController::ExecuteSimTask()
{
    assert(_simTask);
    _simTask(_now);
}

void ParticipantController::ChangeState(ParticipantState newState, std::string reason)
{
    _status.state = newState;
    _status.enterReason = reason;
    _status.enterTime = std::chrono::system_clock::now();

    SendIbMessage(_status);
}
    
    
} // namespace sync
} // namespace mw
} // namespace ib
