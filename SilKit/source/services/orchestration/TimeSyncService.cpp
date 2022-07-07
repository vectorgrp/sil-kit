// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cassert>
#include <future>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "SynchronizedHandlers.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace Orchestration {

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
            // ignore already known participants
            return;
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
    virtual void ReceiveNextSimTask(const Core::IServiceEndpoint* from, const NextSimTask& task) = 0;
};

//! brief Synchronization policy for unsynchronized participants
struct UnsynchronizedPolicy : public ITimeSyncPolicy
{
public:
    UnsynchronizedPolicy() {}
    void Initialize() override {}
    void RequestInitialStep() override {}
    void RequestNextStep() override {}
    void ReceiveNextSimTask(const Core::IServiceEndpoint* /*from*/, const NextSimTask& /*task*/) override {}
};

//! brief Synchronization policy of the VAsio middleware
struct SynchronizedPolicy : public ITimeSyncPolicy
{
public:
    SynchronizedPolicy(TimeSyncService& controller, Core::IParticipantInternal* participant, TimeConfiguration* configuration)
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
        _controller.SendMsg(_configuration->_myNextTask);
        // Bootstrap checked execution, in case there is no other participant.
        // Else, checked execution is initiated when we receive their NextSimTask messages.
        _participant->ExecuteDeferred([this]() {
            this->CheckDistributedTimeAdvanceGrant();
        });
    }

    void RequestNextStep() override { _controller.SendMsg(_configuration->_myNextTask); }

    void ReceiveNextSimTask(const Core::IServiceEndpoint* from, const NextSimTask& task) override
    {
        _configuration->_otherNextTasks[from->GetServiceDescriptor().GetParticipantName()] = task;

        switch (_controller.State())
        {
        case ParticipantState::Invalid: // [[fallthrough]]
        case ParticipantState::ServicesCreated: // [[fallthrough]]
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
    Core::IParticipantInternal* _participant;
    TimeConfiguration* _configuration;
};

TimeSyncService::TimeSyncService(Core::IParticipantInternal* participant, ITimeProvider* timeProvider,
                                 const Config::HealthCheck& healthCheckConfig)
    : _participant{participant}
    , _lifecycleService{nullptr}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
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

    ConfigureTimeProvider(TimeProviderKind::NoSync);
    _timeConfiguration = std::make_shared<TimeConfiguration>();

    participant->GetServiceDiscovery()->RegisterServiceDiscoveryHandler([&](auto, const Core::ServiceDescriptor& descriptor) {
            if (descriptor.GetServiceType() == Core::ServiceType::InternalController)
        {
            std::string controllerType;
            descriptor.GetSupplementalDataItem(Core::Discovery::controllerType, controllerType);
            if (controllerType == Core::Discovery::controllerTypeTimeSyncService)
            {
                std::string timeSyncActive;
                descriptor.GetSupplementalDataItem(Core::Discovery::timeSyncActive, timeSyncActive);
                if (timeSyncActive == "1")
                {
                    auto descriptorParticipantName = descriptor.GetParticipantName();
                    if (descriptorParticipantName == _participant->GetParticipantName())
                    {
                        // ignore self
                        return;
                    }
                    _timeConfiguration->SynchronizedParticipantAdded(descriptorParticipantName);
                }
            }
        }
    });
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
    _lifecycleService->ChangeState(ParticipantState::Error, errorMsg);
}

bool TimeSyncService::IsSynchronized()
{
    return _isSynchronized;
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
    _timeSyncConfigured = true;
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

void TimeSyncService::ReceiveSilKitMessage(const IServiceEndpoint* /*from*/, const ParticipantCommand& command)
{
    if (command.participant != _serviceDescriptor.GetParticipantId())
        return;

    //Initialize(command, std::string{"Received ParticipantCommand::"} + to_string(command.kind));
}

void TimeSyncService::ReceiveSilKitMessage(const IServiceEndpoint* from, const NextSimTask& task)
{
    if (_timeSyncPolicy)
    {
        _timeSyncPolicy->ReceiveNextSimTask(from, task);
    }
}

void TimeSyncService::ReceiveSilKitMessage(const IServiceEndpoint*, const SystemCommand& command)
{
    if (command.kind == SystemCommand::Kind::Run && _timeSyncConfigured)
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
    _isSynchronized = isSynchronized;
    if (_timeSyncPolicy != nullptr)
    {
        return;
    }

    try
    {
        _timeSyncPolicy = MakeTimeSyncPolicy(isSynchronized);
        _serviceDescriptor.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncActive, (isSynchronized) ? "1" : "0");
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

void TimeSyncService::ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind)
{
    _timeProvider->ConfigureTimeProvider(timeProviderKind);
}

auto TimeSyncService::Now() const -> std::chrono::nanoseconds
{
    return _timeProvider->Now();
}

void TimeSyncService::SetLifecycleService(LifecycleService* lifecycleService)
{
    _lifecycleService = lifecycleService;
}
} // namespace Orchestration
} // namespace Services
} // namespace SilKit
