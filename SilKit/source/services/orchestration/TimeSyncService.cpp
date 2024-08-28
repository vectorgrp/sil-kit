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
#include "silkit/services/orchestration/ISystemMonitor.hpp"

#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "ILoggerInternal.hpp"
#include "SynchronizedHandlers.hpp"
#include "Assert.hpp"
#include "VAsioCapabilities.hpp"

using namespace std::chrono_literals;

namespace {
#ifdef _WIN32
auto GetDefaultTimerResolution() -> std::chrono::nanoseconds
{
    return 16ms;
}
#else
auto GetDefaultTimerResolution() -> std::chrono::nanoseconds
{
    return 1ms;
}
#endif
} // namespace

namespace SilKit {
namespace Services {
namespace Orchestration {

struct ITimeSyncPolicy
{
public:
    virtual ~ITimeSyncPolicy() = default;
    virtual void Initialize() = 0;
    virtual void RequestNextStep() = 0;
    virtual void SetSimStepCompleted() = 0;
    virtual auto IsExecutingSimStep() -> bool = 0;
    virtual void ReceiveNextSimTask(const Core::IServiceEndpoint* from, const NextSimTask& task) = 0;
    virtual void ProcessSimulationTimeUpdate() = 0;
};

//! brief Synchronization policy for unsynchronized participants
struct UnsynchronizedPolicy : public ITimeSyncPolicy
{
public:
    UnsynchronizedPolicy() {}
    void Initialize() override {}
    void RequestNextStep() override {}
    void SetSimStepCompleted() override {}
    auto IsExecutingSimStep() -> bool override
    {
        return false;
    }
    void ReceiveNextSimTask(const Core::IServiceEndpoint* /*from*/, const NextSimTask& /*task*/) override {}
    void ProcessSimulationTimeUpdate() override {};
};

//! brief Synchronization policy of the VAsio middleware
struct SynchronizedPolicy : public ITimeSyncPolicy
{
public:
    SynchronizedPolicy(TimeSyncService& controller, Core::IParticipantInternal* participant,
                       TimeConfiguration* configuration)
        : _controller(controller)
        , _participant(participant)
        , _configuration(configuration)
    {
    }

    void Initialize() override
    {
        _configuration->Initialize();
    }

    void SetSimStepCompleted() override
    {
        // after completing the SimTask in Async mode, reset the _isExecutingSimStep guard
        _isExecutingSimStep = false;
    }

    auto IsExecutingSimStep() -> bool override
    {
        return _isExecutingSimStep;
    }

    void RequestNextStep() override
    {
        // ensure that calls to Stop()/Pause() in a SimTask won't send out a new step and eventually call the SimTask again
        if (_controller.State() == ParticipantState::Running && !_controller.StopRequested()
            && !_controller.PauseRequested())
        {
            if (_lastSentNextSimTask
                != _configuration->NextSimStep().timePoint) // Prevent sending same step more than once
            {
                _lastSentNextSimTask = _configuration->NextSimStep().timePoint;
                _controller.SendMsg(_configuration->NextSimStep());
            }
            // Bootstrap checked execution, in case there is no other participant.
            // Else, checked execution is initiated when we receive their NextSimTask messages.
            _participant->ExecuteDeferred([this]() { this->ProcessSimulationTimeUpdate(); });
        }
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
            ProcessSimulationTimeUpdate();
            return;
        case ParticipantState::Stopping: // [[fallthrough]]
        case ParticipantState::Stopped: // [[fallthrough]]
        case ParticipantState::Error: // [[fallthrough]]
        case ParticipantState::ShuttingDown: // [[fallthrough]]
        case ParticipantState::Shutdown: // [[fallthrough]]
            return;
        default:
            _participant->GetLifecycleService()->ReportError("Received NextSimTask in state ParticipantState::"
                                                             + to_string(_controller.State()));
            return;
        }
    }

    void ProcessSimulationTimeUpdate() override
    {
        // Check if we meet the conditions to trigger our local time advancement
        if (IsTimeAdvancePossible())
        {
            if (IsSimStepSync())
            {
                AdvanceTimeSimStepSync();
            }
            else
            {
                AdvanceTimeSimStepAsync();
            }
        }
    }

private:
    bool IsSimStepSync() const
    {
        return _configuration->IsBlocking();
    }

    bool IsTimeAdvancePossible()
    {
        // Deferred execution of this callback was initiated, but simulation stopped/paused in the meantime
        if (_controller.State() != ParticipantState::Running)
        {
            return false;
        }

        // State check is not enough is user called Stop() in the SimTask and directly receives a NextSimTask
        if (_controller.StopRequested() || _controller.PauseRequested())
        {
            return false;
        }

        // Another coordinated participant has disconnected without gracefully shutting down
        if (static_cast<LifecycleService*>(_participant->GetLifecycleService())->GetOperationMode()
                == OperationMode::Coordinated
            && _participant->GetSystemMonitor()->SystemState() == SystemState::Error)
        {
            return false;
        }

        if (_configuration->OtherParticipantHasLowerTimepoint())
        {
            return false;
        }

        // With real-time sync, the time advance is not possible if less then current real-time.
        if (_controller.IsCoupledToWallClock())
        {
            if (_configuration->NextSimStep().timePoint > _controller.GetCurrentWallClockSyncPoint())
            {
                return false;
            }
        }

        // No other participant has a lower time point: It is our turn
        return true;
    }

    void AdvanceTimeSimStepSync()
    {
        AdvanceTimeAndExecuteSimStep();

        // If paused, don't request the next sim step. This happens in LifecycleService::Continue()
        if (!_controller.PauseRequested())
        {
            // With real-time sync, the participant must sigal his readiness for the next step only if the real-time has actually passed.
            if (!_controller.IsCoupledToWallClock())
            {
                // The synchronous SimStep API creates the nextSimTask message automatically after the callback.
                RequestNextStep();
            }
        }
    }

    void AdvanceTimeSimStepAsync()
    {
        // when running in Async mode, set the _isExecutingSimStep guard
        // which will be cleared in CompleteSimulationStep()
        auto test = false;
        auto newval = true;
        if (!_isExecutingSimStep.compare_exchange_strong(test, newval))
        {
            //_isExecutingSimStep was not modified, it was already true
            return;
        }

        AdvanceTimeAndExecuteSimStep();

        // Do nothing until a user calls CompleteSimulationStep()
        // This ensures that only one async SimStep is executed until completed by the user
    }

    void AdvanceTimeAndExecuteSimStep()
    {
        if (_controller.State() == ParticipantState::Paused || _controller.State() == ParticipantState::Running)
        {
            if (_configuration->HandleHopOn())
            {
                if (_controller.AbortHopOnForCoordinatedParticipants())
                {
                    // Prevent that the sim task is triggered
                    return;
                }
            }

            // update the current and next sim. step timestamps
            _configuration->AdvanceTimeStep();
            // Execute the simulation step callback with the current simulation time
            auto currentStep = _configuration->CurrentSimStep();
            _controller.ExecuteSimStep(currentStep.timePoint, currentStep.duration);
        }
    }

    std::atomic<bool> _isExecutingSimStep{false};
    TimeSyncService& _controller;
    std::chrono::nanoseconds _lastSentNextSimTask{-1ns};
    Core::IParticipantInternal* _participant;
    TimeConfiguration* _configuration;
};

TimeSyncService::TimeSyncService(Core::IParticipantInternal* participant, ITimeProvider* timeProvider,
                                 const Config::HealthCheck& healthCheckConfig, LifecycleService* lifecycleService,
                                 double animationFactor)
    : _participant{participant}
    , _lifecycleService{lifecycleService}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _timeConfiguration{participant->GetLogger()}
    , _simStepCounterMetric{participant->GetMetricsManager()->GetCounter("SimStepCount")}
    , _simStepExecutionTimeStatisticMetric{participant->GetMetricsManager()->GetStatistic("SimStepExecutionDuration")}
    , _simStepWaitingTimeStatisticMetric{participant->GetMetricsManager()->GetStatistic("SimStepWaitingDuration")}
    , _watchDog{healthCheckConfig}
    , _animationFactor{animationFactor}
{
    _isCoupledToWallClock = _animationFactor != 0.0;
    if (_isCoupledToWallClock)
    {
        Debug(_logger, "TimeSyncService: Coupled to the local wall clock with animation factor {}", _animationFactor);
    }

    _watchDog.SetWarnHandler([logger = _logger](std::chrono::milliseconds timeout) {
        Warn(logger, "SimStep did not finish within soft time limit. Timeout detected after {} ms",
             std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count());
    });
    _watchDog.SetErrorHandler([this](std::chrono::milliseconds timeout) {
        std::stringstream buffer;
        buffer << "SimStep did not finish within hard time limit. Timeout detected after "
               << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(timeout).count() << "ms";
        _lifecycleService->ReportError(buffer.str());
    });

    ConfigureTimeProvider(TimeProviderKind::NoSync);

    participant->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
        [&](auto discoveryEventType, const Core::ServiceDescriptor& descriptor) {
        if (descriptor.GetServiceType() == Core::ServiceType::InternalController)
        {
            std::string controllerType;
            descriptor.GetSupplementalDataItem(Core::Discovery::controllerType, controllerType);
            if (controllerType == Core::Discovery::controllerTypeTimeSyncService)
            {
                auto descriptorParticipantName = descriptor.GetParticipantName();
                if (descriptorParticipantName == _participant->GetParticipantName())
                {
                    // ignore self
                    return;
                }

                std::string timeSyncActive;
                descriptor.GetSupplementalDataItem(Core::Discovery::timeSyncActive, timeSyncActive);
                if (timeSyncActive == "1")
                {
                    if (discoveryEventType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                    {
                        // Check capabilities of newly discovered participants.
                        // This might happen before TimeSyncService and LifecycleService are finally configured,
                        // so this check happens also in TimeSyncService::StartTime()
                        if (!ParticipantHasAutonomousSynchronousCapability(descriptorParticipantName))
                        {
                            _participant->GetSystemController()->AbortSimulation();
                            return;
                        }

                        Debug(_participant->GetLogger(),
                              "TimeSyncService: Participant \'{}\' is added to the distributed time synchronization",
                              descriptorParticipantName);

                        _timeConfiguration.AddSynchronizedParticipant(descriptorParticipantName);

                        // If our time has advanced, we just added a late-joining participant.
                        if (_timeConfiguration.CurrentSimStep().timePoint >= 0ns)
                        {
                            // Resend our NextSimTask again because it is not assured that the late-joiner has seen our last update.
                            // At this point, the late-joiner will receive it because its TimeSyncPolicy is configured when the
                            // discovery arrives that triggered this handler.
                            Debug(_participant->GetLogger(),
                                  "Participant \'{}\' is joining an already running simulation. Resending our "
                                  "NextSimTask.",
                                  descriptorParticipantName);
                            SendMsg(_timeConfiguration.NextSimStep());
                        }
                    }
                    else if (discoveryEventType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved)
                    {
                        // Other participant hopped off
                        if (_timeConfiguration.RemoveSynchronizedParticipant(descriptorParticipantName))
                        {
                            Debug(_logger,
                                  "TimeSyncService: Participant '{}' is no longer part of the "
                                  "distributed time synchronization.",
                                  descriptorParticipantName);

                            if (_timeSyncPolicy)
                            {
                                // _otherNextTasks has changed, check if our sim task is due
                                GetTimeSyncPolicy()->ProcessSimulationTimeUpdate();
                            }
                        }
                    }
                }
            }
        }
    });
}

TimeSyncService::~TimeSyncService()
{
    StopWallClockCouplingThread();
}

bool TimeSyncService::IsSynchronizingVirtualTime()
{
    return _isSynchronizingVirtualTime;
}

auto TimeSyncService::State() const -> ParticipantState
{
    return _lifecycleService->State();
}

auto TimeSyncService::StopRequested() const -> bool
{
    return _lifecycleService->StopRequested();
}
auto TimeSyncService::PauseRequested() const -> bool
{
    return _lifecycleService->PauseRequested();
}

auto TimeSyncService::IsCoupledToWallClock() const -> bool
{
    return _isCoupledToWallClock;
}

void TimeSyncService::RequestNextStep()
{
    _participant->ExecuteDeferred([this] { GetTimeSyncPolicy()->RequestNextStep(); });
}

void TimeSyncService::SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize)
{
    _simTask = std::move(task);
    _timeConfiguration.SetBlockingMode(true);
    _timeConfiguration.SetStepDuration(initialStepSize);
}

void TimeSyncService::SetSimulationStepHandlerAsync(SimulationStepHandler task,
                                                    std::chrono::nanoseconds initialStepSize)
{
    _simTask = std::move(task);
    _timeConfiguration.SetBlockingMode(false);
    _timeConfiguration.SetStepDuration(initialStepSize);
}

void TimeSyncService::SetPeriod(std::chrono::nanoseconds period)
{
    _timeConfiguration.SetStepDuration(period);
}

bool TimeSyncService::SetupTimeSyncPolicy(bool isSynchronizingVirtualTime)
{
    std::lock_guard<decltype(_timeSyncPolicyMx)> lock{_timeSyncPolicyMx};

    if (_timeSyncPolicy != nullptr)
    {
        return false;
    }

    _timeSyncConfigured = true;
    if (isSynchronizingVirtualTime)
    {
        _timeSyncPolicy = std::make_shared<SynchronizedPolicy>(*this, _participant, &_timeConfiguration);
    }
    else
    {
        _timeSyncPolicy = std::make_shared<UnsynchronizedPolicy>();
    }

    return true;
}

void TimeSyncService::ReceiveMsg(const IServiceEndpoint* from, const NextSimTask& task)
{
    const auto timeSyncPolicy = GetTimeSyncPolicy();
    if (timeSyncPolicy != nullptr)
    {
        timeSyncPolicy->ReceiveNextSimTask(from, task);
    }
    else
    {
        Logging::Debug(_logger, "Received NextSimTask from participant \'{}\' but TimeSyncPolicy is not yet configured",
                       from->GetServiceDescriptor().GetParticipantName());
    }
}

void TimeSyncService::ExecuteSimStep(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration)
{
    SILKIT_ASSERT(_simTask);
    using DoubleMSecs = std::chrono::duration<double, std::milli>;
    using DoubleSecs = std::chrono::duration<double>;

    _waitTimeMonitor.StopMeasurement();
    const auto waitingDuration = _waitTimeMonitor.CurrentDuration();
    const auto waitingDurationMs = std::chrono::duration_cast<DoubleMSecs>(waitingDuration);
    const auto waitingDurationS = std::chrono::duration_cast<DoubleSecs>(waitingDuration);

    Trace(_logger, "Starting next Simulation Task. Waiting time was: {}ms", waitingDurationMs.count());
    if (_waitTimeMonitor.SampleCount() > 1)
    {
        // skip the first sample, since it was never 'started' (it is always the current epoch of the underlying clock)
        _simStepWaitingTimeStatisticMetric->Take(waitingDurationS.count());
    }

    _timeProvider->SetTime(timePoint, duration);

    _execTimeMonitor.StartMeasurement();
    _watchDog.Start();
    _simTask(timePoint, duration);
    _watchDog.Reset();
    _execTimeMonitor.StopMeasurement();

    const auto executionDuration = _execTimeMonitor.CurrentDuration();
    const auto executionDurationMs = std::chrono::duration_cast<DoubleMSecs>(executionDuration);
    const auto executionDurationS = std::chrono::duration_cast<DoubleSecs>(executionDuration);

    _simStepCounterMetric->Add(1);
    _simStepExecutionTimeStatisticMetric->Take(executionDurationS.count());

    Trace(_logger, "Finished Simulation Step. Execution time was: {}ms", executionDurationMs.count());
    _waitTimeMonitor.StartMeasurement();
}

void TimeSyncService::CompleteSimulationStep()
{
    _logger->Debug("CompleteSimulationStep: calling _timeSyncPolicy->RequestNextStep");
    _participant->ExecuteDeferred([this] {
        GetTimeSyncPolicy()->SetSimStepCompleted();

        // With real-time sync, the next step is requested in the real-time thread. If lagging behind, request also here to catch up.
        if (!_isCoupledToWallClock || _wallClockReachedBeforeCompletion)
        {
            _wallClockReachedBeforeCompletion = false;
            GetTimeSyncPolicy()->RequestNextStep();
        }
    });
}

//! \brief Create a time provider that caches the current simulation time.
void TimeSyncService::InitializeTimeSyncPolicy(bool isSynchronizingVirtualTime)
{
    _isSynchronizingVirtualTime = isSynchronizingVirtualTime;
    _timeProvider->SetSynchronizeVirtualTime(isSynchronizingVirtualTime);

    try
    {
        // create and assign time sync policy
        const auto timeSyncPolicyCreated = SetupTimeSyncPolicy(isSynchronizingVirtualTime);
        if (timeSyncPolicyCreated == false)
        {
            return;
        }

        _serviceDescriptor.SetSupplementalDataItem(SilKit::Core::Discovery::timeSyncActive,
                                                   (isSynchronizingVirtualTime) ? "1" : "0");
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
    GetTimeSyncPolicy()->Initialize();
}

void TimeSyncService::ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind)
{
    _timeProvider->ConfigureTimeProvider(timeProviderKind);
}

void TimeSyncService::StartTime()
{
    if (_timeSyncConfigured)
    {
        const auto timeSyncPolicy = GetTimeSyncPolicy();
        SILKIT_ASSERT(timeSyncPolicy);
        if (_isSynchronizingVirtualTime)
        {
            // Check if all synchronous participants have the necessary capabilities
            bool missingCapability = false;
            for (auto&& participantName : _timeConfiguration.GetSynchronizedParticipantNames())
            {
                if (!ParticipantHasAutonomousSynchronousCapability(participantName))
                {
                    missingCapability = true;
                }
            }
            if (missingCapability)
            {
                _participant->GetSystemController()->AbortSimulation();
            }
        }

        if (_isCoupledToWallClock)
        {
            StartWallClockCouplingThread();
        }

        // Start the distributed time algorithm by sending our NextSimStep
        GetTimeSyncPolicy()->RequestNextStep();
    }
}

void TimeSyncService::StopTime()
{
    if (_isCoupledToWallClock)
    {
        StopWallClockCouplingThread();
    }
}

auto TimeSyncService::Now() const -> std::chrono::nanoseconds
{
    return _timeProvider->Now();
}

auto TimeSyncService::GetTimeConfiguration() -> TimeConfiguration*
{
    return &_timeConfiguration;
}

bool TimeSyncService::ParticipantHasAutonomousSynchronousCapability(const std::string& participantName) const
{
    if (_lifecycleService && _lifecycleService->GetOperationMode() == OperationMode::Autonomous
        && _lifecycleService->IsTimeSyncActive()
        && !_participant->ParticipantHasCapability(participantName, SilKit::Core::Capabilities::AutonomousSynchronous))
    {
        // We are a participant with autonomous lifecycle and virtual time sync.
        // The remote participant must support this, otherwise Hop-On / Hop-Off will fail.
        Error(_participant->GetLogger(),
              "Participant \'{}\' does not support simulations with participants that use an autonomous lifecycle "
              "and virtual time synchronization. Please consider upgrading Participant \'{}\'. Aborting simulation...",
              participantName, participantName);
        return false;
    }
    return true;
}

bool TimeSyncService::AbortHopOnForCoordinatedParticipants() const
{
    if (_lifecycleService)
    {
        if (_lifecycleService->GetOperationMode() == OperationMode::Coordinated)
        {
            Error(_participant->GetLogger(),
                  "This participant is running with a coordinated lifecycle and virtual time synchronization and wants "
                  "to join an already running simulation. This is not allowed, aborting simulation...");
            _participant->GetSystemController()->AbortSimulation();
            return true;
        }
    }
    return false;
}

auto TimeSyncService::GetCurrentWallClockSyncPoint() const -> std::chrono::nanoseconds
{
    return std::chrono::nanoseconds{_currentWallClockSyncPointNs};
}


// Mixture of sleep_for and busy waiting to achieve higher precision sleeps with the low-res windows times
void TimeSyncService::HybridWait(std::chrono::nanoseconds targetWaitDuration)
{
    auto busyWait = [](std::chrono::nanoseconds duration) {
        auto startTime = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - startTime < duration)
        {
            std::this_thread::yield();
        }
    };

    // By default, Windows timer have a resolution of 15.6ms
    // The effective time that wait_for or sleep_for actually waits will be a step function with steps every 15.6ms
    const auto defaultTimerResolution = GetDefaultTimerResolution();

    if (targetWaitDuration < defaultTimerResolution)
    {
        busyWait(targetWaitDuration);
    }
    else
    {
        auto timeBeforeSleep = std::chrono::steady_clock::now();
        // Wait the share of the targetWaitDuration that we can precisely achieve with the low timer resolution
        auto sleepDuration = targetWaitDuration - defaultTimerResolution;
        std::this_thread::sleep_for(sleepDuration);
        // Busy-wait the remainder
        auto remainder = targetWaitDuration - (std::chrono::steady_clock::now() - timeBeforeSleep);
        busyWait(remainder);
    }
}


void TimeSyncService::StartWallClockCouplingThread()
{
    _wallClockCouplingThreadRunning = true;
    _wallClockCouplingThread = std::thread{[this]() {
        SilKit::Util::SetThreadName("SK-WallClkSync");

        const auto startTime = std::chrono::steady_clock::now();
        auto nextAnimatedWallClockSyncPoint = _timeConfiguration.NextSimStep().duration * _animationFactor;

        while (_wallClockCouplingThreadRunning)
        {
            // Wait until the next SimTask is due according to the (animated) wall clock
            auto currentRunDuration = std::chrono::steady_clock::now() - startTime;
            auto durationUntilNextWallClockSyncPoint = nextAnimatedWallClockSyncPoint - currentRunDuration;
            HybridWait(std::chrono::duration_cast<std::chrono::nanoseconds>(durationUntilNextWallClockSyncPoint));

            if (State() == ParticipantState::Running)
            {
                auto duration =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(_timeConfiguration.NextSimStep().duration);
                _currentWallClockSyncPointNs += duration.count();
                nextAnimatedWallClockSyncPoint += duration * _animationFactor;

                if (GetTimeSyncPolicy()->IsExecutingSimStep())
                {
                    // AsyncSimStepHandler not completed? Execution is lagging behind. Don't send the NextSimStep now, but after completion.
                    _logger->Warn("Simulation step was not completed in time to achieve wall clock coupling.");
                    _wallClockReachedBeforeCompletion = true;
                }
                else
                {
                    _participant->ExecuteDeferred([this]() { GetTimeSyncPolicy()->RequestNextStep(); });
                }
            }
        }
    }};
}

void TimeSyncService::StopWallClockCouplingThread()
{
    if (_wallClockCouplingThreadRunning)
    {
        _wallClockCouplingThreadRunning = false;
        if (_wallClockCouplingThread.joinable())
        {
            _wallClockCouplingThread.join();
        }
    }
}

bool TimeSyncService::IsBlocking() const
{
    return _timeConfiguration.IsBlocking();
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
