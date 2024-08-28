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

#pragma once

#include <future>
#include <tuple>
#include <map>
#include <atomic>

#include "silkit/services/orchestration/ITimeSyncService.hpp"

#include "IMsgForTimeSyncService.hpp"
#include "IParticipantInternal.hpp"
#include "LifecycleService.hpp"
#include "ParticipantConfiguration.hpp"
#include "PerformanceMonitor.hpp"
#include "TimeProvider.hpp"
#include "TimeConfiguration.hpp"
#include "WatchDog.hpp"
#include "Metrics.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//forward declarations
class SynchronizedVirtualTimeProvider;
class TimeConfiguration;

struct ITimeSyncPolicy;

class TimeSyncService
    : public ITimeSyncService
    , public IMsgForTimeSyncService
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    TimeSyncService(Core::IParticipantInternal* participant, ITimeProvider* timeProvider,
                    const Config::HealthCheck& healthCheckConfig, LifecycleService* lifecycleService,
                    double animationFactor = 0);

    ~TimeSyncService();

public:
    // ----------------------------------------
    // Public Methods
    // ITimeSyncService
    auto State() const -> ParticipantState;
    void SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override;
    void SetSimulationStepHandlerAsync(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) override;
    void CompleteSimulationStep() override;
    void SetPeriod(std::chrono::nanoseconds period);
    void ReceiveMsg(const IServiceEndpoint* from, const NextSimTask& task) override;
    auto Now() const -> std::chrono::nanoseconds override;

    // Used by Policies
    template <class MsgT>
    void SendMsg(MsgT&& msg) const;
    void ExecuteSimStep(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    void InitializeTimeSyncPolicy(bool isSynchronizingVirtualTime);
    void ResetTime();
    void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind);
    void StartTime();
    void StopTime();

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    bool IsSynchronizingVirtualTime();

    // To add other participants in unit tests
    auto GetTimeConfiguration() -> TimeConfiguration*;

    bool ParticipantHasAutonomousSynchronousCapability(const std::string& participantName) const;
    bool AbortHopOnForCoordinatedParticipants() const;

    auto StopRequested() const -> bool;
    auto PauseRequested() const -> bool;

    void RequestNextStep();

    auto IsCoupledToWallClock() const -> bool;
    auto GetCurrentWallClockSyncPoint() const -> std::chrono::nanoseconds;

    bool IsBlocking() const;

private:
    // ----------------------------------------
    // private methods

    //! Creates the _timeSyncPolicy. Returns true if the call assigned the _timeSyncPolicy, and false if it was already
    //! assigned before.
    bool SetupTimeSyncPolicy(bool isSynchronizingVirtualTime);

    inline auto GetTimeSyncPolicy() const -> ITimeSyncPolicy*;

    void StopWallClockCouplingThread();
    void StartWallClockCouplingThread();
    void HybridWait(std::chrono::nanoseconds targetWaitDuration);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    LifecycleService* _lifecycleService{nullptr};
    Services::Logging::ILogger* _logger{nullptr};
    ITimeProvider* _timeProvider{nullptr};
    TimeConfiguration _timeConfiguration;

    VSilKit::ICounterMetric* _simStepCounterMetric;
    VSilKit::IStatisticMetric* _simStepExecutionTimeStatisticMetric;
    VSilKit::IStatisticMetric* _simStepWaitingTimeStatisticMetric;

    mutable std::mutex _timeSyncPolicyMx;
    std::shared_ptr<ITimeSyncPolicy> _timeSyncPolicy{nullptr};

    std::vector<std::string> _requiredParticipants;

    bool _isRunning{false};
    bool _isSynchronizingVirtualTime{false};
    bool _timeSyncConfigured{false};

    SimulationStepHandler _simTask;
    std::future<void> _asyncResult;

    Util::PerformanceMonitor _execTimeMonitor;
    Util::PerformanceMonitor _waitTimeMonitor;
    WatchDog _watchDog;
    bool _isCoupledToWallClock{false};
    std::thread _wallClockCouplingThread;
    mutable std::mutex _mx;
    std::atomic<std::chrono::nanoseconds::rep> _currentWallClockSyncPointNs{0};
    double _animationFactor{0};
    std::atomic<bool> _wallClockCouplingThreadRunning{false};
    std::atomic<bool> _wallClockReachedBeforeCompletion{false};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void TimeSyncService::SendMsg(MsgT&& msg) const
{
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}

void TimeSyncService::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto TimeSyncService::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto TimeSyncService::GetTimeSyncPolicy() const -> ITimeSyncPolicy*
{
    std::unique_lock<decltype(_timeSyncPolicyMx)> lock{_timeSyncPolicyMx};
    return _timeSyncPolicy.get();
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
