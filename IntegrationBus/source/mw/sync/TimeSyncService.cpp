// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"

#include <cassert>
#include <future>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

class TimeConfiguration
{
public:
    TimeConfiguration() 
        : _blocking(false)
    {
        _currentTask.timePoint = -1ns;
        _currentTask.duration = 0ns;
        _myNextTask.timePoint = 0ns;
        // NB: This is used when SetPeriod is never called
        _myNextTask.duration = 1ms;
    }

    void SetBlockingMode(bool blocking) { _blocking = blocking; }

    void SynchronizedParticipantAdded(const std::string& otherParticipantName)
    {
        if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
        {
            const std::string errorMessage{"Participant " + otherParticipantName + " already known."};
            throw std::runtime_error{errorMessage};
        }
        NextSimTask task;
        task.timePoint = -1ns;
        task.duration = 0ns;
        _otherNextTasks[otherParticipantName] = task;
    }

    void SynchronizedParticipantRemoved(const std::string& otherParticipantName)
    {
        if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
        {
            const std::string errorMessage{"Participant " + otherParticipantName + " unknown."};
            throw std::runtime_error{errorMessage};
        }
        auto it = _otherNextTasks.find(otherParticipantName);
        if (it != _otherNextTasks.end())
        {
            _otherNextTasks.erase(it);
        }
    }
    void SetStepDuration(std::chrono::nanoseconds duration) { _myNextTask.duration = duration; }

    // TODO make private and create getters/setters
public:
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;
};

struct ITimeSyncPolicy
{
public:
    virtual ~ITimeSyncPolicy() = default;
    virtual void Initialize() = 0;
    virtual void RequestInitialStep() = 0;
    virtual void RequestNextStep() = 0;
    virtual void ReceiveNextSimTask(const IIbServiceEndpoint* from, const NextSimTask& task) = 0;
};

//! brief Synchronization policy for unsynchronized participants
struct UnsynchronizedPolicy : public ITimeSyncPolicy
{
public:
    UnsynchronizedPolicy() {}
    void Initialize() override {}
    void RequestInitialStep() override {}
    void RequestNextStep() override {}
    void ReceiveNextSimTask(const IIbServiceEndpoint* /*from*/, const NextSimTask& /*task*/) override {}
};

//! brief Synchronization policy of the VAsio middleware
struct SynchronizedPolicy : public ITimeSyncPolicy
{
public:
    SynchronizedPolicy(TimeSyncService& controller, IParticipantInternal* participant, TimeConfiguration* configuration)
        : _controller(controller)
        , _participant(participant)
        , _configuration(configuration)
    {
        _configuration->SetBlockingMode(true);
    }

    void Initialize() override
    {
        _configuration->_currentTask.timePoint = -1ns;
        _configuration->_currentTask.duration = 0ns;
        _configuration->_myNextTask.timePoint = 0ns;
    }

    void RequestInitialStep() override
    {
        _controller.SendIbMessage(_configuration->_myNextTask);
        // Bootstrap checked execution, in case there is no other participant.
        // Else, checked execution is initiated when we receive their NextSimTask messages.
        _participant->ExecuteDeferred([this]() {
            this->CheckDistributedTimeAdvanceGrant();
        });
    }

    void RequestNextStep() override { _controller.SendIbMessage(_configuration->_myNextTask); }

    void ReceiveNextSimTask(const IIbServiceEndpoint* from, const NextSimTask& task) override
    {
        _configuration->_otherNextTasks[from->GetServiceDescriptor().GetParticipantName()] = task;

        switch (_controller.State())
        {
        case ParticipantState::Invalid: // [[fallthrough]]
        case ParticipantState::ControllersCreated: // [[fallthrough]]
        case ParticipantState::CommunicationInitializing: // [[fallthrough]]
        case ParticipantState::CommunicationInitialized: // [[fallthrough]]
        case ParticipantState::ReadyToRun: return;
        case ParticipantState::Paused: // [[fallthrough]]
        case ParticipantState::Running: CheckDistributedTimeAdvanceGrant(); return;
        case ParticipantState::Stopping: // [[fallthrough]]
        case ParticipantState::Stopped: // [[fallthrough]]
        case ParticipantState::Error: // [[fallthrough]]
        case ParticipantState::ShuttingDown: // [[fallthrough]]
        case ParticipantState::Shutdown: // [[fallthrough]]
            return;
        default:
            _controller.ReportError("Received NextSimTask in state ParticipantState::"
                                    + to_string(_controller.State()));
            return;
        }
    }

private:
    void CheckDistributedTimeAdvanceGrant()
    {
        // Deferred execution of this callback was initiated, but simulation stopped in the meantime
        if (_controller.State() != ParticipantState::Running)
        {
            return;
        }

        for (auto&& otherTask : _configuration->_otherNextTasks)
        {
            if (_configuration->_myNextTask.timePoint > otherTask.second.timePoint)
                return;
        }

        // No other participant has a lower time point: It is our turn
        _configuration->_currentTask = _configuration->_myNextTask;
        _configuration->_myNextTask.timePoint =
            _configuration->_currentTask.timePoint + _configuration->_currentTask.duration;
        _controller.ExecuteSimTask(_configuration->_currentTask.timePoint, _configuration->_currentTask.duration);
        _controller.AwaitNotPaused();
        if (_configuration->_blocking)
        {
            //NB: CompleteSimulationTask does invoke this explicitly on the API caller's request:
            RequestNextStep();
        }

        for (auto&& otherTask : _configuration->_otherNextTasks)
        {
            if (_configuration->_myNextTask.timePoint > otherTask.second.timePoint)
                return;
        }

        // Still, no other participant has a lower time point: Check again later
        _participant->ExecuteDeferred([this]() {
            this->CheckDistributedTimeAdvanceGrant();
        });
    }

    TimeSyncService& _controller;
    IParticipantInternal* _participant;
    TimeConfiguration* _configuration;
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

    const std::string& TimeProviderName() const override { return _name; }

    void RegisterNextSimStepHandler(NextSimStepHandlerT handler) override
    {
        _handlers.emplace_back(std::move(handler));
    }
};

TimeSyncService::TimeSyncService(IParticipantInternal* participant, LifecycleService* lifecycleService,
                                 const cfg::HealthCheck& healthCheckConfig)
    : _participant{participant}
    , _lifecycleService{lifecycleService}
    , _logger{participant->GetLogger()}
    , _watchDog{healthCheckConfig}
{
    _watchDog.SetWarnHandler([logger = _logger](std::chrono::milliseconds timeout) {
        logger->Warn("SimTask did not finish within soft time limit. Timeout detected after {} ms",
                     std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count());
    });
    _watchDog.SetErrorHandler([this](std::chrono::milliseconds timeout) {
        std::stringstream buffer;
        buffer << "SimTask did not finish within hard time limit. Timeout detected after "
               << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count() << "ms";
        this->ReportError(buffer.str());
    });

    _timeProvider = std::make_shared<ParticipantTimeProvider>();
    _timeConfiguration = std::make_shared<TimeConfiguration>();
}

void TimeSyncService::ReportError(const std::string& errorMsg)
{
    _logger->Error(errorMsg);

    if (State() == ParticipantState::Shutdown)
    {
        _logger->Warn("TimeSyncService::ReportError() was called in terminal state ParticipantState::Shutdown; "
                      "transition to ParticipantState::Error is ignored.");
        return;
    }
    _lifecycleService->ChangeState(ParticipantState::Error, std::move(errorMsg));
}

void TimeSyncService::AddSynchronizedParticipants(const ExpectedParticipants& expectedParticipants)
{
    //if (_isSynchronized)
    {
        // TODO check this - seems odd...
        auto&& nameIter =
            std::find(expectedParticipants.names.begin(), expectedParticipants.names.end(), _lifecycleService->Status().participantName);
        if (nameIter == expectedParticipants.names.end())
        {
            std::stringstream strs;
            strs << "Synchronized participant " << _lifecycleService->Status().participantName
                 << " not found in expected participants.";
            throw std::runtime_error{strs.str()};
        }

        // Add sync participants
        for (auto&& name : expectedParticipants.names)
        {
            // Exclude this participant
            if (name == _participant->GetParticipantName())
            {
                continue;
            }
            _timeConfiguration->SynchronizedParticipantAdded(name);
        }
    }
}

auto TimeSyncService::State() const -> ParticipantState
{
    return _lifecycleService->Status().state;
}

void TimeSyncService::SetSimulationTask(
    std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task)
{
    _simTask = std::move(task);
    _timeConfiguration->SetBlockingMode(true);
}

void TimeSyncService::SetSimulationTaskAsync(SimTaskT task)
{
    _simTask = std::move(task);
    _timeConfiguration->SetBlockingMode(false);
}

void TimeSyncService::SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task)
{
    _simTask = [task = std::move(task)](auto now, auto /*duration*/) {
        task(now);
    };
    _timeConfiguration->SetBlockingMode(true);
}

void TimeSyncService::SetPeriod(std::chrono::nanoseconds period)
{
    _timeConfiguration->SetStepDuration(period);
}

auto TimeSyncService::MakeTimeSyncPolicy(bool isSynchronized) -> std::shared_ptr<ITimeSyncPolicy>
{
    if (isSynchronized)
    {
        // TODO FIXME get on shared pointer?
        return std::make_shared<SynchronizedPolicy>(*this, _participant, _timeConfiguration.get());
    }
    else
    {
        return std::make_shared<UnsynchronizedPolicy>();
    }
}

void TimeSyncService::SetPaused(std::future<void> pausedFuture)
{
    _pauseDone = std::move(pausedFuture);
}

void TimeSyncService::AwaitNotPaused()
{
    if (_lifecycleService->State() == ParticipantState::Paused)
    {
        _pauseDone.wait();
    }
}

auto TimeSyncService::Now() const -> std::chrono::nanoseconds
{
    return _timeProvider->Now();
}

void TimeSyncService::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ParticipantCommand& command)
{
    // TODO FIXME VIB-551
    if (command.participant != _serviceDescriptor.GetParticipantId())
        return;

    //Initialize(command, std::string{"Received ParticipantCommand::"} + to_string(command.kind));
}

void TimeSyncService::ReceiveIbMessage(const IIbServiceEndpoint* from, const NextSimTask& task)
{
    if (_timeSyncPolicy)
    {
        _timeSyncPolicy->ReceiveNextSimTask(from, task);
    }
}

void TimeSyncService::ReceiveIbMessage(const IIbServiceEndpoint*, const SystemCommand& command)
{
    if (command.kind == SystemCommand::Kind::Run)
    {
        assert(_timeSyncPolicy);
        _timeSyncPolicy->RequestInitialStep();
    }
}

void TimeSyncService::ExecuteSimTask(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration)
{
    assert(_simTask);
    using DoubleMSecs = std::chrono::duration<double, std::milli>;

    _waitTimeMonitor.StopMeasurement();
    _logger->Trace("Starting next Simulation Task. Waiting time was: {}ms",
                   std::chrono::duration_cast<DoubleMSecs>(_waitTimeMonitor.CurrentDuration()).count());

    _timeProvider->SetTime(timePoint, duration);

    _execTimeMonitor.StartMeasurement();
    _watchDog.Start();
    _simTask(timePoint, duration);
    _watchDog.Reset();
    _execTimeMonitor.StopMeasurement();

    _logger->Trace("Finished Simulation Task. Execution time was: {}ms",
                   std::chrono::duration_cast<DoubleMSecs>(_execTimeMonitor.CurrentDuration()).count());
    _waitTimeMonitor.StartMeasurement();
}

void TimeSyncService::CompleteSimulationTask()
{
    _logger->Debug("CompleteSimulationTask: calling _timeSyncPolicy->RequestNextStep");
    _timeSyncPolicy->RequestNextStep();
}

//! \brief Create a time provider that caches the current simulation time.
void TimeSyncService::InitializeTimeSyncPolicy(bool isSynchronized)
{
    if (_timeSyncPolicy != nullptr)
    {
        return;
    }

    try
    {
        _timeSyncPolicy = MakeTimeSyncPolicy(isSynchronized);
        _serviceDescriptor.SetSupplementalDataItem(ib::mw::service::timeSyncActive, std::to_string(isSynchronized));
        ResetTime();
    }
    catch (const std::exception& e)
    {
        _logger->Critical(e.what());
        throw;
    }
}

void TimeSyncService::ResetTime()
{
    _timeSyncPolicy->Initialize();
}

auto TimeSyncService::GetTimeProvider() -> std::shared_ptr<sync::ITimeProvider>
{
    return _timeProvider;
}

} // namespace sync
} // namespace mw
} // namespace ib
