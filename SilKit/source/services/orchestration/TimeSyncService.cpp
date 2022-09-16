/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <future>
#include <functional>
#include <atomic>

#include "silkit/services/orchestration/string_utils.hpp"

#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "ILogger.hpp"
#include "SynchronizedHandlers.hpp"
#include "Assert.hpp"

using namespace std::chrono_literals;
namespace SilKit {
namespace Services {
namespace Orchestration {

struct ITimeSyncPolicy
{
public:
    virtual ~ITimeSyncPolicy() = default;
    virtual void Initialize() = 0;
    virtual void RequestInitialStep() = 0;
    virtual void RequestNextStep() = 0;
    virtual void SetSimStepCompleted() = 0;
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
    void SetSimStepCompleted() override {}
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
    }

    void Initialize() override
    {
        _configuration->Initialize();
    }

    void RequestInitialStep() override
    {
        _controller.SendMsg(_configuration->NextSimStep());
        // Bootstrap checked execution, in case there is no other participant.
        // Else, checked execution is initiated when we receive their NextSimTask messages.
        _participant->ExecuteDeferred([this]() {
            this->CheckDistributedTimeAdvanceGrant();
        });
    }

    void SetSimStepCompleted() override
    {
        // after completing the SimTask in Async mode, reset the _isExecutingSimStep guard
        _isExecutingSimStep = false;
    }

    void RequestNextStep() override
    {
        _controller.SendMsg(_configuration->NextSimStep());
        _participant->ExecuteDeferred([this]() {
            this->CheckDistributedTimeAdvanceGrant();
        });
    }

    void ReceiveNextSimTask(const Core::IServiceEndpoint* from, const NextSimTask& task) override
    {
        _configuration->OnReceiveNextSimStep(from->GetServiceDescriptor().GetParticipantName(), task);

        switch (_controller.State())
        {
        case ParticipantState::Invalid: // [[fallthrough]]
        case ParticipantState::ServicesCreated: // [[fallthrough]]
        case ParticipantState::CommunicationInitializing: // [[fallthrough]]
        case ParticipantState::CommunicationInitialized: // [[fallthrough]]
        case ParticipantState::ReadyToRun:
            return;
        case ParticipantState::Paused: // [[fallthrough]]
        case ParticipantState::Running:
            CheckDistributedTimeAdvanceGrant();
            return;
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
    bool IsAsync() const
    {
        return !IsSync();
    }

    bool IsSync() const
    {
        return _configuration->IsBlocking();
    }

    void CheckDistributedTimeAdvanceGrant()
    {
        // Deferred execution of this callback was initiated, but simulation stopped in the meantime
        if (_controller.State() != ParticipantState::Running)
        {
            return;
        }

        if (_configuration->OtherParticipantHasLowerTimepoint())
        {
            return;
        }

        // when running in Async mode, set the _isExecutingSimStep guard
        // which will be cleared in CompleteSimulationStep()
        if (IsAsync())
        {
            auto test = false;
            auto newval = true;
            if(!_isExecutingSimStep.compare_exchange_strong(test, newval))
            {
                //_isExecutingSimStep was not modified, it was already true
                return;
            }
        }

        // No other participant has a lower time point: It is our turn
        _configuration->AdvanceTimeStep();
        auto currentStep = _configuration->CurrentSimStep();
        _controller.ExecuteSimStep(currentStep.timePoint, currentStep.duration);
        _controller.AwaitNotPaused();

        if (IsSync())
        {
            //NB: CompleteSimulationStep does invoke this explicitly on the API caller's request:
            RequestNextStep();
        }
        else
        {
            // Do nothing until a user calls CompleteSimulationStep()
            // This ensures that only one async SimTask is executed until completed by the user
            return;
        }
    }

    std::atomic<bool> _isExecutingSimStep{false};
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
        Warn(logger, "SimTask did not finish within soft time limit. Timeout detected after {} ms",
                     std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count());
    });
    _watchDog.SetErrorHandler([this](std::chrono::milliseconds timeout) {
        std::stringstream buffer;
        buffer << "SimTask did not finish within hard time limit. Timeout detected after "
               << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count() << "ms";
        this->ReportError(buffer.str());
    });

    ConfigureTimeProvider(TimeProviderKind::NoSync);

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
                    _timeConfiguration.SynchronizedParticipantAdded(descriptorParticipantName);
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
        Warn(_logger, "TimeSyncService::ReportError() was called in terminal state ParticipantState::Shutdown; "
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

void TimeSyncService::SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize)
{
    _simTask = std::move(task);
    _timeConfiguration.SetBlockingMode(true);
    _timeConfiguration.SetStepDuration(initialStepSize);
}

void TimeSyncService::SetSimulationStepHandlerAsync(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize)
{
    _simTask = std::move(task);
    _timeConfiguration.SetBlockingMode(false);
    _timeConfiguration.SetStepDuration(initialStepSize);
}

void TimeSyncService::SetPeriod(std::chrono::nanoseconds period)
{
    _timeConfiguration.SetStepDuration(period);
}

auto TimeSyncService::MakeTimeSyncPolicy(bool isSynchronized) -> std::shared_ptr<ITimeSyncPolicy>
{
    _timeSyncConfigured = true;
    if (isSynchronized)
    {
        return std::make_shared<SynchronizedPolicy>(*this, _participant, &_timeConfiguration);
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

void TimeSyncService::ReceiveMsg(const IServiceEndpoint* from, const NextSimTask& task)
{
    if (_timeSyncPolicy)
    {
        _timeSyncPolicy->ReceiveNextSimTask(from, task);
    }
}

void TimeSyncService::ExecuteSimStep(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration)
{
    SILKIT_ASSERT(_simTask);
    using DoubleMSecs = std::chrono::duration<double, std::milli>;

    _waitTimeMonitor.StopMeasurement();
    Trace(_logger, "Starting next Simulation Task. Waiting time was: {}ms",
                   std::chrono::duration_cast<DoubleMSecs>(_waitTimeMonitor.CurrentDuration()).count());

    _timeProvider->SetTime(timePoint, duration);

    _execTimeMonitor.StartMeasurement();
    _watchDog.Start();
    _simTask(timePoint, duration);
    _watchDog.Reset();
    _execTimeMonitor.StopMeasurement();

    Trace(_logger, "Finished Simulation Task. Execution time was: {}ms",
                   std::chrono::duration_cast<DoubleMSecs>(_execTimeMonitor.CurrentDuration()).count());
    _waitTimeMonitor.StartMeasurement();
}

void TimeSyncService::CompleteSimulationStep()
{
    _logger->Debug("CompleteSimulationStep: calling _timeSyncPolicy->RequestNextStep");
    _timeSyncPolicy->SetSimStepCompleted();
    _timeSyncPolicy->RequestNextStep();
}

//! \brief Create a time provider that caches the current simulation time.
void TimeSyncService::InitializeTimeSyncPolicy(bool isSynchronized)
{
    _isSynchronized = isSynchronized;
    _timeProvider->SetSynchronized(isSynchronized);

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

void TimeSyncService::StartTime()
{
    if (_timeSyncConfigured)
    {
        SILKIT_ASSERT(_timeSyncPolicy);
        _timeSyncPolicy->RequestInitialStep();
    }
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
