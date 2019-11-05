// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantController.hpp"

#include <cassert>
#include <future>

#include "ib/cfg/string_utils.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

struct DistributedTimeQuantumAdapter : ParticipantController::ISyncAdapter
{
    void RequestStep(ParticipantController& controller) override
    {
        controller.SendNextSimTask();
    }
    void FinishedStep(ParticipantController& /*controller*/) override {}
};

struct TimeQuantumAdapter : ParticipantController::ISyncAdapter
{
    void RequestStep(ParticipantController& controller) override
    {
        controller.SendQuantumRequest();
    }
    void FinishedStep(ParticipantController& /*controller*/) override {}
};

struct DiscreteTimeAdapter : ParticipantController::ISyncAdapter
{
    void RequestStep(ParticipantController& /*controller*/) override {}
    void FinishedStep(ParticipantController& controller) override
    {
        controller.SendTickDone();
    }
};

struct DiscreteTimePassiveAdapter : ParticipantController::ISyncAdapter
{
    void RequestStep(ParticipantController& /*controller*/) override {}
    void FinishedStep(ParticipantController& /*controller*/) override {}
};

ParticipantController::ParticipantController(IComAdapter* comAdapter, const cfg::SimulationSetup& simulationSetup, const cfg::Participant& participantConfig)
    : _comAdapter{comAdapter}
    , _timesyncConfig{simulationSetup.timeSync}
    , _syncType{participantConfig.participantController->syncType}
    , _logger{comAdapter->GetLogger()}
    , _watchDog{participantConfig.participantController->execTimeLimitSoft, participantConfig.participantController->execTimeLimitHard}
{
    _watchDog.SetWarnHandler(
        [logger = _logger](std::chrono::milliseconds timeout)
        {
            logger->Warn("SimTask did not finish within soft limit. Timeout detected after {} ms",
                std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count());
        }
    );
    _watchDog.SetErrorHandler(
        [this](std::chrono::milliseconds timeout)
        {
            std::stringstream buffer;
            buffer
                << "SimTask did not finish within hard limit. Timeout detected after "
                << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count()
                << "ms";
            this->ReportError(buffer.str());
        }
    );


    _status.participantName = participantConfig.name;
    _currentTask.timePoint = -1ns;
    _currentTask.duration = 0ns;
    _myNextTask.timePoint = 0ns;
    _myNextTask.duration = _timesyncConfig.tickPeriod;

    for (auto&& participant : simulationSetup.participants)
    {
        if (participant.name == participantConfig.name)
            continue;

        if (participant.participantController->syncType == cfg::SyncType::DistributedTimeQuantum)
        {
            NextSimTask task;
            task.timePoint = -1ns;
            task.duration = 0ns;
            _otherNextTasks[participant.id] = task;
        }
    }
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

void ParticipantController::SetSimulationTask(std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task)
{
    _simTask = std::move(task);
}

void ParticipantController::SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task)
{
    _simTask = [task = std::move(task)](auto now, auto /*duration*/){ task(now); };
}

void ParticipantController::EnableColdswap()
{
    _coldswapEnabled = true;
}

void ParticipantController::SetPeriod(std::chrono::nanoseconds period)
{
    if (_syncType != cfg::SyncType::TimeQuantum)
    {
        auto msPeriod = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(period);
        _logger->Warn("ParticipantController::SetPeriod({}ms) is ignored", msPeriod.count());
        _logger->Info("ParticipantController::SetPeriod() can only be used with SyncType::TimeQuantum (currently active: SyncType::{})", _syncType);
    }
    _myNextTask.duration = period;
}

void ParticipantController::SetEarliestEventTime(std::chrono::nanoseconds /*eventTime*/)
{
    throw std::exception();
}

auto ParticipantController::MakeSyncAdapter(ib::cfg::SyncType syncType) -> std::unique_ptr<ParticipantController::ISyncAdapter>
{
    switch (syncType)
    {
    case cfg::SyncType::DistributedTimeQuantum:
        return std::make_unique<DistributedTimeQuantumAdapter>();
    case cfg::SyncType::DiscreteEvent:
        throw std::runtime_error("Unsupported SyncType " + to_string(syncType));
    case cfg::SyncType::TimeQuantum:
        return std::make_unique<TimeQuantumAdapter>();
    case cfg::SyncType::DiscreteTime:
        return std::make_unique<DiscreteTimeAdapter>();
    case cfg::SyncType::DiscreteTimePassive:
        return std::make_unique<DiscreteTimePassiveAdapter>();
    default:
        throw ib::cfg::Misconfiguration("Invalid SyncType " + to_string(syncType));
    }
}

auto ParticipantController::Run() -> ParticipantState
{
    return RunAsync().get();
}

auto ParticipantController::RunAsync() -> std::future<ParticipantState>
{
    if (!_simTask)
    {
        const std::string errorMsg{"ParticipantController::Run() was called without having set a SimTask!"};
        ReportError(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    try
    {
        _syncAdapter = MakeSyncAdapter(_syncType);
    }
    catch (const std::exception& e)
    {
        _logger->Critical(e.what());
        throw;
    }

    ChangeState(ParticipantState::Idle, "ParticipantController::Run() was called");
    return _finalStatePromise.get_future();
}

void ParticipantController::ReportError(std::string errorMsg)
{
    _logger->Error(errorMsg);

    if (State() == ParticipantState::Shutdown)
    {
        _logger->Warn("ParticipantController::ReportError() was called in terminal state ParticipantState::Shutdown; transition to ParticipantState::Error is ignored.");
        return;
    }
    ChangeState(ParticipantState::Error, std::move(errorMsg));
}

void ParticipantController::Pause(std::string reason)
{
    if (State() != ParticipantState::Running)
    {
        const std::string errorMessage{"ParticipantController::Pause() was called in state ParticipantState::" + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    ChangeState(ParticipantState::Paused, std::move(reason));
}

void ParticipantController::Continue()
{
    if (State() != ParticipantState::Paused)
    {
        const std::string errorMessage{"ParticipantController::Continue() was called in state ParticipantState::" + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    ChangeState(ParticipantState::Running, "Pause finished");
}

void ParticipantController::Initialize(const ParticipantCommand& command, std::string reason)
{
    ChangeState(ParticipantState::Initializing, reason);
    if (_initHandler)
    {
        try
        {
            _initHandler(command);
            reason += " and InitHandler completed without exception";
        }
        catch (const std::exception& e)
        {
            const std::string errorMessage{"InitHandler did throw an exception: " + std::string{e.what()}};
            ReportError(errorMessage);
            return;
        }
    }
    else
    {
        reason += " and no InitHandler was registered";
    }

    _currentTask.timePoint = -1ns;
    _currentTask.duration = 0ns;
    _myNextTask.timePoint = 0ns;
    ChangeState(ParticipantState::Initialized, std::move(reason));
}

void ParticipantController::Stop(std::string reason)
{
    ChangeState(ParticipantState::Stopping, reason);

    if (_stopHandler)
    {
        try
        {
            _stopHandler();
            // The handler can report an error, which overrules the default transition to ParticipantState::Stopped
            if (State() != ParticipantState::Error)
            {
                reason += " and StopHandler completed successfully";
                ChangeState(ParticipantState::Stopped, std::move(reason));
            }
        }
        catch (const std::exception& e)
        {
            reason += " and StopHandler threw exception: ";
            reason += e.what();
            ChangeState(ParticipantState::Stopped, std::move(reason));
        }
    }
    else
    {
        reason += " and no StopHandler registered";
        ChangeState(ParticipantState::Stopped, reason);
    }

}

void ParticipantController::Shutdown(std::string reason)
{
    ChangeState(ParticipantState::ShuttingDown, reason);

    if (_shutdownHandler)
    {
        try
        {
            _shutdownHandler();
            reason += " and ShutdownHandler completed";
            ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
        catch (const std::exception& e)
        {
            reason += " and ShutdownHandler threw exception: ";
            reason += e.what();
            ChangeState(ParticipantState::Shutdown, std::move(reason));
        }
    }
    else
    {
        reason += " and no ShutdownHandler was registered";
        ChangeState(ParticipantState::Shutdown, std::move(reason));
    }

    _finalStatePromise.set_value(State());
}

void ParticipantController::PrepareColdswap()
{
    _logger->Info("preparing coldswap...");
    ChangeState(ParticipantState::ColdswapPrepare, "Starting coldswap preparations");

    _comAdapter->OnAllMessagesDelivered([this]() {

        _comAdapter->FlushSendBuffers();
        ChangeState(ParticipantState::ColdswapReady, "Finished coldswap preparations.");
        _logger->Info("ready for coldswap...");

    });
}

void ParticipantController::ShutdownForColdswap()
{
    _comAdapter->FlushSendBuffers();
    ChangeState(ParticipantState::ColdswapShutdown, "Coldswap was enabled for this participant.");

    _comAdapter->OnAllMessagesDelivered([this]() {

        _finalStatePromise.set_value(State());

    });
}

void ParticipantController::IgnoreColdswap()
{
    _comAdapter->FlushSendBuffers();
    ChangeState(ParticipantState::ColdswapIgnored, "Coldswap was not enabled for this participant.");
}

auto ParticipantController::State() const -> ParticipantState
{
    return _status.state;
}

auto ParticipantController::Status() const -> const ParticipantStatus&
{
    return _status;
}

void ParticipantController::RefreshStatus()
{
    _status.refreshTime = std::chrono::system_clock::now();
    SendIbMessage(_status);
}

auto ParticipantController::Now() const -> std::chrono::nanoseconds
{
    return _currentTask.timePoint;
}

void ParticipantController::LogCurrentPerformanceStats()
{
    using DoubleMSecs = std::chrono::duration<double, std::milli>;
    auto toMSecs = [](auto duration) { return std::chrono::duration_cast<DoubleMSecs>(duration).count(); };

    if (_execTimeMonitor.SampleCount() == 0u)
    {
        _logger->Info("TotalTaskTime: -.--ms [-.--, -.--] \tWaitTime: -.--ms [-.--, -.--]  \tCpuTime: -.--ms, [-.--, -.--] \t(avg [min,max])");
    }
    else
    {
        _logger->Info("TotalTaskTime: {:.2f}ms [{:.2f}, {:.2f}] \tWaitTime: {:.2f}ms [{:.2f}, {:.2f}]  \tCpuTime: {:.2f}, [{:.2f}, {:.2f}] \t(avg [min,max])",
            toMSecs(_execTimeMonitor.AvgDuration<DoubleMSecs>() + _waitTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_execTimeMonitor.MinDuration() + _waitTimeMonitor.MinDuration()),
            toMSecs(_execTimeMonitor.MaxDuration() + _waitTimeMonitor.MaxDuration()),

            toMSecs(_waitTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_waitTimeMonitor.MinDuration()),
            toMSecs(_waitTimeMonitor.MaxDuration()),

            toMSecs(_execTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_execTimeMonitor.MinDuration()),
            toMSecs(_execTimeMonitor.MaxDuration())
        );
    }
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

    Initialize(command, std::string{"Received ParticipantCommand::"} + to_string(command.kind));
}

void ParticipantController::ReceiveIbMessage(ib::mw::EndpointAddress /*from*/, const SystemCommand& command)
{
    // We have to supress a SystemCommand::ExecuteColdswap during the restart
    // After a coldswap, this command is still present in the SystemControllers
    // history and thus retransmitted to the reconnecting participant. However,
    // we cannot flush this change from the SystemController's history because
    // this could happen before the command has been received by the participant,
    // thus missing the command.
    if (command.kind == SystemCommand::Kind::ExecuteColdswap && (State() == ParticipantState::Invalid || State() == ParticipantState::Idle))
    {
        return;
    }

    if (!_syncAdapter)
    {
        ReportError("Received SystemCommand::" + to_string(command.kind) + " before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (command.kind)
    {
    case SystemCommand::Kind::Invalid:
        break;

    case SystemCommand::Kind::Run:
        if (State() == ParticipantState::Initialized)
        {
            ChangeState(ParticipantState::Running, "Received SystemCommand::Run");
            _waitTimeMonitor.StartMeasurement();
            _syncAdapter->RequestStep(*this);
            return;
        }
        break;

    case SystemCommand::Kind::Stop:
        if (State() == ParticipantState::Stopped)
        {
            _logger->Warn("Received SystemCommand::Stop, but ignored since already ParticipantState::Stopped");
            return;
        }
        else if (State() == ParticipantState::Running)
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

    case SystemCommand::Kind::PrepareColdswap:
        if (State() == ParticipantState::Error || State() == ParticipantState::Stopped)
        {
            PrepareColdswap();
            return;
        }
        break;

    case SystemCommand::Kind::ExecuteColdswap:
        if (State() == ParticipantState::ColdswapReady)
        {
            if (_coldswapEnabled)
            {
                ShutdownForColdswap();
            }
            else
            {
                IgnoreColdswap();
            }
            return;
        }
        break;
    }

    // We should not reach this point in normal operation.
    ReportError("Received SystemCommand::" + to_string(command.kind) + " while in ParticipantState::" + to_string(State()));
}

void ParticipantController::ReceiveIbMessage(mw::EndpointAddress /*from*/, const Tick& msg)
{
    switch (_syncType)
    {
    case cfg::SyncType::DiscreteTime:
    case cfg::SyncType::DiscreteTimePassive:
        break;
    default:
        return;
    }

    if (!_syncAdapter)
    {
        ReportError("Received TICK before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (State())
    {
    // TICKs during initialization are considered as an error
    case ParticipantState::Invalid:      // [[fallthrough]]
    case ParticipantState::Idle:         // [[fallthrough]]
    case ParticipantState::Initializing: // [[fallthrough]]
    case ParticipantState::Initialized:
        ReportError("Received TICK in state ParticipantState::" + to_string(State()));
        return;

    case ParticipantState::Paused:
        // We have to process TICKS received in Paused. This can happen due to
        // race conditions, and we can't undo a TICK. It should not occur more
        // than once though.
        // [[fallthrough]]
    case ParticipantState::Running:
        _currentTask.timePoint = msg.now;
        _currentTask.duration = msg.duration;
        ExecuteSimTask();
        break;

    // TICK during stop/shutdown procedure and Errors are ignored
    case ParticipantState::Stopping:     // [[fallthrough]]
    case ParticipantState::Stopped:      // [[fallthrough]]
    case ParticipantState::Error:        // [[fallthrough]]
    case ParticipantState::ShuttingDown: // [[fallthrough]]
    case ParticipantState::Shutdown:     // [[fallthrough]]
        return;
    default:
        ReportError("Received TICK in state ParticipantState::" + to_string(State()));
        return;
    }
}

void ParticipantController::ReceiveIbMessage(mw::EndpointAddress /*from*/, const QuantumGrant& msg)
{
    if (_syncType != cfg::SyncType::TimeQuantum)
        return;

    if (_endpointAddress != msg.grantee)
        return;

    if (!_syncAdapter)
    {
        ReportError("Received QuantumGrant before ParticipantController::Run() or RunAsync() was called");
        return;
    }

    switch (State())
    {
    // QuantumGrant during initialization are considered as an error
    case ParticipantState::Invalid:      // [[fallthrough]]
    case ParticipantState::Idle:         // [[fallthrough]]
    case ParticipantState::Initializing: // [[fallthrough]]
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

    // QuantumGrant during stop/shutdown procedure and Errors are ignored
    case ParticipantState::Stopping:     // [[fallthrough]]
    case ParticipantState::Stopped:      // [[fallthrough]]
    case ParticipantState::Error:        // [[fallthrough]]
    case ParticipantState::ShuttingDown: // [[fallthrough]]
    case ParticipantState::Shutdown:     // [[fallthrough]]
        return;

    default:
        ReportError("Received QuantumGrant in state ParticipantState::" + to_string(State()));
        return;
    }
}

void ParticipantController::ReceiveIbMessage(mw::EndpointAddress from, const NextSimTask& task)
{
    if (from == _endpointAddress) return;

    _otherNextTasks[from.participant] = task;

    switch (State())
    {
        // QuantumGrant during initialization are considered as an error
    case ParticipantState::Invalid:      // [[fallthrough]]
    case ParticipantState::Idle:         // [[fallthrough]]
    case ParticipantState::Initializing: // [[fallthrough]]
    case ParticipantState::Initialized:
        return;

    case ParticipantState::Paused:
        // We have to process QuantumGrants received in Paused. This can happen
        // due to race conditions, and we can't undo a TICK. It should not occur
        // more than once though.
        // [[fallthrough]]
    case ParticipantState::Running:
        CheckDistributedTimeAdvanceGrant();
        return;

        // QuantumGrant during stop/shutdown procedure and Errors are ignored
    case ParticipantState::Stopping:     // [[fallthrough]]
    case ParticipantState::Stopped:      // [[fallthrough]]
    case ParticipantState::Error:        // [[fallthrough]]
    case ParticipantState::ShuttingDown: // [[fallthrough]]
    case ParticipantState::Shutdown:     // [[fallthrough]]
        return;

    default:
        ReportError("Received NextSimTask in state ParticipantState::" + to_string(State()));
        return;
    }
}

void ParticipantController::SendTickDone() const
{
    if (_timesyncConfig.syncPolicy == cfg::TimeSync::SyncPolicy::Strict)
    {
        _comAdapter->OnAllMessagesDelivered([this]() {

            SendIbMessage(TickDone{Tick{_currentTask.timePoint, _currentTask.duration}});

        });
    }
    else
    {
        SendIbMessage(TickDone{Tick{_currentTask.timePoint, _currentTask.duration}});
    }
}

void ParticipantController::SendQuantumRequest() const
{
    if (_timesyncConfig.syncPolicy == cfg::TimeSync::SyncPolicy::Strict)
    {
        _comAdapter->OnAllMessagesDelivered([this]() {

            SendIbMessage(QuantumRequest{_myNextTask.timePoint, _myNextTask.duration});

        });
    }
    else
    {
        SendIbMessage(QuantumRequest{_myNextTask.timePoint, _myNextTask.duration});
    }
}

void ParticipantController::ProcessQuantumGrant(const QuantumGrant& msg)
{
    switch (msg.status)
    {
    case QuantumRequestStatus::Granted:
        if (msg.now != _myNextTask.timePoint || msg.duration != _myNextTask.duration)
        {
            ReportError("Granted quantum duration does not match request!");
        }
        else
        {
            _currentTask = _myNextTask;
            _myNextTask.timePoint = _currentTask.timePoint + _currentTask.duration;
            ExecuteSimTask();
        }
        break;
    case QuantumRequestStatus::Rejected:
        break;
    case QuantumRequestStatus::Invalid:
        ReportError("Received invalid QuantumGrant");
        break;
    default:
        ReportError("Received QuantumGrant with unknown Status");
    }
}

void ParticipantController::SendNextSimTask()
{
    if (_timesyncConfig.syncPolicy == cfg::TimeSync::SyncPolicy::Strict)
    {
        _comAdapter->OnAllMessagesDelivered([this]() {

            SendIbMessage(_myNextTask);

        });
    }
    else
    {
        SendIbMessage(_myNextTask);
    }
}

void ParticipantController::CheckDistributedTimeAdvanceGrant()
{
    while (true)
    {
        for (auto&& otherTask : _otherNextTasks)
        {
            if (_myNextTask.timePoint > otherTask.second.timePoint)
                return;
        }

        // No SimTask has a lower timePoint
        _currentTask = _myNextTask;
        _myNextTask.timePoint = _currentTask.timePoint + _currentTask.duration;
        ExecuteSimTask();
    }
}

void ParticipantController::ExecuteSimTask()
{
    assert(_simTask);
    using DoubleMSecs = std::chrono::duration<double, std::milli>;

    _waitTimeMonitor.StopMeasurement();
    _logger->Trace("Starting next Simulation Task. Waiting time was: {}ms", std::chrono::duration_cast<DoubleMSecs>(_waitTimeMonitor.CurrentDuration()).count());

    _execTimeMonitor.StartMeasurement();
    _watchDog.Start();
    _simTask(_currentTask.timePoint, _currentTask.duration);
    _watchDog.Reset();
    _execTimeMonitor.StopMeasurement();

    _logger->Trace("Finished Simulation Task. Execution time was: {}ms", std::chrono::duration_cast<DoubleMSecs>(_execTimeMonitor.CurrentDuration()).count());
    _waitTimeMonitor.StartMeasurement();

    _syncAdapter->FinishedStep(*this);
    _syncAdapter->RequestStep(*this);
}

void ParticipantController::ChangeState(ParticipantState newState, std::string reason)
{
    _status.state = newState;
    _status.enterReason = reason;
    _status.enterTime = std::chrono::system_clock::now();
    _status.refreshTime = _status.enterTime;

    SendIbMessage(_status);
}


} // namespace sync
} // namespace mw
} // namespace ib
