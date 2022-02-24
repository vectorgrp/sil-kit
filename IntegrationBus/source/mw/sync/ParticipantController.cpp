// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ParticipantController.hpp"
#include "IServiceDiscovery.hpp"

#include <cassert>
#include <future>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

//! brief Synchronization policy for unsynchronized participants
struct UnsynchronizedPolicy : ParticipantController::ITimeSyncPolicy
{
    UnsynchronizedPolicy()
    {
    }
    void Initialize() override { }
    void SynchronizedParticipantAdded(const std::string& otherParticipantName) override { }
    void SynchronizedParticipantRemoved(const std::string& otherParticipantName) override { }
    void RequestInitialStep() override { }
    void RequestNextStep() override { }
    void SetStepDuration(std::chrono::nanoseconds duration) override { }
    void ReceiveNextSimTask(const IIbServiceEndpoint* from, const NextSimTask& task) override { }
    void SetBlockingMode(bool) override { }
};

//! brief Synchronization policy of the VAsio middleware
struct DistributedTimeQuantumPolicy : ParticipantController::ITimeSyncPolicy
{
    DistributedTimeQuantumPolicy(ParticipantController& controller, IComAdapterInternal* comAdapter)
        : _controller(controller)
        , _comAdapter(comAdapter)
        , _blocking(true)
    {
        _currentTask.timePoint = -1ns;
        _currentTask.duration = 0ns;
        _myNextTask.timePoint = 0ns;
        // NB: This is used when SetPeriod is never called
        _myNextTask.duration = 1ms;
    }

    void Initialize() override
    {
        _currentTask.timePoint = -1ns;
        _currentTask.duration = 0ns;
        _myNextTask.timePoint = 0ns;
    }

    void SynchronizedParticipantAdded(const std::string& otherParticipantName) override
    {
        if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
        {
            const std::string errorMessage{ "Participant " + otherParticipantName + " already known."};
            throw std::runtime_error { errorMessage };
        }
        NextSimTask task;
        task.timePoint = -1ns;
        task.duration = 0ns;
        _otherNextTasks[otherParticipantName] = task;
    }

    void SynchronizedParticipantRemoved(const std::string& otherParticipantName) override
    {
        if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
        {
            const std::string errorMessage{"Participant " + otherParticipantName + " unknown." };
            throw std::runtime_error{errorMessage};
        }
        auto it = _otherNextTasks.find(otherParticipantName);
        if (it != _otherNextTasks.end())
        {
            _otherNextTasks.erase(it);
        }
    }

    void RequestInitialStep() override
    {
        _controller.SendIbMessage(_myNextTask);
        // Bootstrap checked execution, in case there is no other participant.
        // Else, checked execution is initiated when we receive their NextSimTask messages.
        _comAdapter->ExecuteDeferred([this]() { this->CheckDistributedTimeAdvanceGrant(); });
    }

    void RequestNextStep() override
    {
        _controller.SendIbMessage(_myNextTask);
    }

    void SetStepDuration(std::chrono::nanoseconds duration) override
    {
        _myNextTask.duration = duration;
    }

    void ReceiveNextSimTask(const IIbServiceEndpoint* from, const NextSimTask& task) override
    {
        _otherNextTasks[from->GetServiceDescriptor().GetParticipantName()] = task;

        switch (_controller.State())
        {
        case ParticipantState::Invalid:      // [[fallthrough]]
        case ParticipantState::Idle:         // [[fallthrough]]
        case ParticipantState::Initializing: // [[fallthrough]]
        case ParticipantState::Initialized:
            return;
        case ParticipantState::Paused:       // [[fallthrough]]
        case ParticipantState::Running:
            CheckDistributedTimeAdvanceGrant();
            return;
        case ParticipantState::Stopping:     // [[fallthrough]]
        case ParticipantState::Stopped:      // [[fallthrough]]
        case ParticipantState::Error:        // [[fallthrough]]
        case ParticipantState::ShuttingDown: // [[fallthrough]]
        case ParticipantState::Shutdown:     // [[fallthrough]]
            return;
        default:
            _controller.ReportError("Received NextSimTask in state ParticipantState::" + to_string(_controller.State())); 
            return;
        }
    }

    void SetBlockingMode(bool newValue) override { _blocking = newValue; }
private:
    void CheckDistributedTimeAdvanceGrant()
    {
        for (auto&& otherTask : _otherNextTasks)
        {
            if (_myNextTask.timePoint > otherTask.second.timePoint)
                return;
        }

        // No other participant has a lower time point: It is our turn
        _currentTask = _myNextTask;
        _myNextTask.timePoint = _currentTask.timePoint + _currentTask.duration;
        if (_blocking)
        {
          _controller.ExecuteSimTask(_currentTask.timePoint, _currentTask.duration);
          _controller.AwaitNotPaused();
          RequestNextStep();
        }
        else
        {
          _controller.ExecuteSimTaskNonBlocking(_currentTask.timePoint, _currentTask.duration);
          _controller.AwaitNotPaused();
          RequestNextStep();
        }

        for (auto&& otherTask : _otherNextTasks)
        {
            if (_myNextTask.timePoint > otherTask.second.timePoint)
                return;
        }

        // Still, no other participant has a lower time point: Check again later
        _comAdapter->ExecuteDeferred([this]() { this->CheckDistributedTimeAdvanceGrant(); });
    }

    ParticipantController& _controller;
    IComAdapterInternal* _comAdapter;
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;
};

//! \brief  A caching time provider: we update its internal state whenever the controller's 
//          simulation time changes.
// This ensures that the our time provider is available even after
// the ParticipantController gets destructed.
struct ParticipantTimeProvider : public sync::ITimeProvider
{
    std::chrono::nanoseconds _now;
    const std::string _name{"ParticipantTimeProvider"};
    std::vector<NextSimStepHandlerT> _handlers;

    auto SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) 
    {
        // tell our users about the next simulation step
        for (auto& handler : _handlers)
            handler(now, duration);
        _now = now; 
    }

    auto Now() const -> std::chrono::nanoseconds override { return _now; }

    const std::string& TimeProviderName() const  override { return _name; }

    void RegisterNextSimStepHandler(NextSimStepHandlerT handler)  override
    {
        _handlers.emplace_back(std::move(handler));
    }
};

ParticipantController::ParticipantController(IComAdapterInternal* comAdapter, const std::string& name,
                                             bool isSynchronized, const cfg::v1::datatypes::HealthCheck& healthCheckConfig)
    : _comAdapter{comAdapter}
    , _isSynchronized{isSynchronized}
    , _logger{comAdapter->GetLogger()}
    , _watchDog{ healthCheckConfig }
{
    _watchDog.SetWarnHandler(
        [logger = _logger](std::chrono::milliseconds timeout)
        {
            logger->Warn("SimTask did not finish within soft time limit. Timeout detected after {} ms",
                std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count());
        }
    );
    _watchDog.SetErrorHandler(
        [this](std::chrono::milliseconds timeout)
        {
            std::stringstream buffer;
            buffer
                << "SimTask did not finish within hard time limit. Timeout detected after "
                << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count()
                << "ms";
            this->ReportError(buffer.str());
        }
    );

    _status.participantName = name;

    try
    {
        _timeSyncPolicy = MakeTimeSyncPolicy(_isSynchronized);
    }
    catch (const std::exception& e)
    {
        _logger->Critical(e.what());
        throw;
    }

    _timeProvider = std::make_shared<ParticipantTimeProvider>();
    _waitingForCompletion = false;
}


void ParticipantController::AddSynchronizedParticipants(const ExpectedParticipants& expectedParticipants)
{
    if (_isSynchronized)
    {
        auto&& nameIter =
            std::find(expectedParticipants.names.begin(), expectedParticipants.names.end(), _status.participantName);
        if (nameIter == expectedParticipants.names.end())
        {
            std::stringstream strs;
            strs << "Synchronized participant " << _status.participantName << " not found in expected participants.";
            throw std::runtime_error{ strs.str() };
        }

        // Add sync participants
        for (auto&& name : expectedParticipants.names)
        {
            // Exclude this participant
            if (name == _status.participantName)
            {
                continue;
            }
            _timeSyncPolicy->SynchronizedParticipantAdded(name);
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
    _timeSyncPolicy->SetBlockingMode(true);
}

void ParticipantController::SetSimulationTaskAsync(SimTaskT task)
{
    _simTask = std::move(task);
    _timeSyncPolicy->SetBlockingMode(false);
}

void ParticipantController::SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task)
{
    _simTask = [task = std::move(task)](auto now, auto /*duration*/){ task(now); };
    _timeSyncPolicy->SetBlockingMode(true);
}

void ParticipantController::EnableColdswap()
{
    _coldswapEnabled = true;
}

void ParticipantController::SetPeriod(std::chrono::nanoseconds period)
{
    _timeSyncPolicy->SetStepDuration(period);
}

auto ParticipantController::MakeTimeSyncPolicy(bool isSynchronized)
    -> std::unique_ptr<ParticipantController::ITimeSyncPolicy>
{
    if (isSynchronized)
    {
        return std::make_unique<DistributedTimeQuantumPolicy>(*this, _comAdapter);
    }
    else
    {
        return std::make_unique<UnsynchronizedPolicy>();
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

    _isRunning = true;
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
    _pauseDonePromise = decltype(_pauseDonePromise){};
    _pauseDone = _pauseDonePromise.get_future();
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
    _pauseDonePromise.set_value();
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

    _timeSyncPolicy->Initialize();
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

void ParticipantController::ForceShutdown(std::string reason)
{
    if (State() != ParticipantState::Stopped)
    {
        _logger->Error("ParticipantController::ForceShutdown() is ignored. Participant is not in state stopped.");
        return;
    }

    _logger->Warn("Executing a forceful shutdown due to: {}", reason);

    std::string shutdownReason = "ParticipantController::ForceShutdown() was called. Reason: " + reason;
    
    Shutdown(shutdownReason);
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

void ParticipantController::AwaitNotPaused()
{
    if (State() == ParticipantState::Paused)
    {
        _pauseDone.wait();
    }
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
    return _timeProvider->Now();
}

void ParticipantController::LogCurrentPerformanceStats()
{
    using DoubleMSecs = std::chrono::duration<double, std::milli>;
    auto toMSecs = [](auto duration) { return std::chrono::duration_cast<DoubleMSecs>(duration).count(); };

    if (_execTimeMonitor.SampleCount() == 0u)
    {
        _logger->Info("\tTotalTaskTime: -.--ms [-.--, -.--, -.--]"
            "\tWaitTime: -.--ms [-.--, -.--, -.--]"
            "\tCpuTime: -.--ms [-.--, -.--, -.--]"
            "\t(cur, [avg,min,max])");
    }
    else
    {
        _logger->Info("\tTotalTaskTime: {:0.2f}ms [{:0.2f}, {:0.2f}, {:0.2f}]"
            "\tWaitTime: {:0.2f}ms [{:0.2f}, {:0.2f}, {:0.2f}]"
            "\tCpuTime: {:0.2f}ms [{:0.2f}, {:0.2f}, {:0.2f}]"
            "\t(cur [avg,min,max])",
            toMSecs(_execTimeMonitor.CurrentDuration() + _waitTimeMonitor.CurrentDuration()),
            toMSecs(_execTimeMonitor.AvgDuration<DoubleMSecs>() + _waitTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_execTimeMonitor.MinDuration() + _waitTimeMonitor.MinDuration()),
            toMSecs(_execTimeMonitor.MaxDuration() + _waitTimeMonitor.MaxDuration()),

            toMSecs(_waitTimeMonitor.CurrentDuration()),
            toMSecs(_waitTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_waitTimeMonitor.MinDuration()),
            toMSecs(_waitTimeMonitor.MaxDuration()),

            toMSecs(_execTimeMonitor.CurrentDuration()),
            toMSecs(_execTimeMonitor.AvgDuration<DoubleMSecs>()),
            toMSecs(_execTimeMonitor.MinDuration()),
            toMSecs(_execTimeMonitor.MaxDuration())
        );
    }
}

void ParticipantController::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ParticipantCommand& command)
{
    // TODO FIXME VIB-551
    if (command.participant != _serviceDescriptor.GetParticipantId())
        return;

    Initialize(command, std::string{"Received ParticipantCommand::"} + to_string(command.kind));
}

void ParticipantController::ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& command)
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

    if (!_isSynchronized)
    {
        return;
    }

    if (!_isRunning)
    {
        std::stringstream msg;
        msg << "Received SystemCommand::"
            << command.kind
            << " before ParticipantController::Run() or RunAsync() was called."
            << " Origin of current command was "
            << from->GetServiceDescriptor()
            ;
        ReportError(msg.str());
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
            _timeSyncPolicy->RequestInitialStep();
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
        else if (State() == ParticipantState::Shutdown || State() == ParticipantState::ShuttingDown)
        {
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

void ParticipantController::ReceiveIbMessage(const IIbServiceEndpoint* from, const NextSimTask& task)
{
    _timeSyncPolicy->ReceiveNextSimTask(from, task);
}

void ParticipantController::ExecuteSimTask(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration)
{
    assert(_simTask);
    using DoubleMSecs = std::chrono::duration<double, std::milli>;

    _waitTimeMonitor.StopMeasurement();
    _logger->Trace("Starting next Simulation Task. Waiting time was: {}ms", std::chrono::duration_cast<DoubleMSecs>(_waitTimeMonitor.CurrentDuration()).count());

    _timeProvider->SetTime(timePoint, duration);

    _execTimeMonitor.StartMeasurement();
    _watchDog.Start();
    _simTask(timePoint, duration);
    _watchDog.Reset();
    _execTimeMonitor.StopMeasurement();

    _logger->Trace("Finished Simulation Task. Execution time was: {}ms", std::chrono::duration_cast<DoubleMSecs>(_execTimeMonitor.CurrentDuration()).count());
    _waitTimeMonitor.StartMeasurement();


}

void ParticipantController::ExecuteSimTaskNonBlocking(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration)
{
    assert(_simTask);
    std::unique_lock<std::mutex> cvLock(_waitingForCompletionMutex);
    if (_waitingForCompletion)
    {
      throw std::runtime_error("SimulationTask already waiting for completion");
    }

    ExecuteSimTask(timePoint, duration);

    _waitingForCompletion = true;
    while (_waitingForCompletion)
        _waitingForCompletionCv.wait_for(cvLock, std::chrono::milliseconds(10));
}

void ParticipantController::CompleteSimulationTask()
{
    std::unique_lock<std::mutex> cvLock(_waitingForCompletionMutex);
    if (!_waitingForCompletion)
    {
      throw std::runtime_error("bad call to CompleteSimulationTask");
    }
    _waitingForCompletion = false;
    _waitingForCompletionCv.notify_all();
}

void ParticipantController::ChangeState(ParticipantState newState, std::string reason)
{
    _status.state = newState;
    _status.enterReason = reason;
    _status.enterTime = std::chrono::system_clock::now();
    _status.refreshTime = _status.enterTime;

    SendIbMessage(_status);
}

//! \brief Create a time provider that caches the current simulation time.
auto ParticipantController::GetTimeProvider() -> std::shared_ptr<sync::ITimeProvider>
{
    return _timeProvider;
}

} // namespace sync
} // namespace mw
} // namespace ib
