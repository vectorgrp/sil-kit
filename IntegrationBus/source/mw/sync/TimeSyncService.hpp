// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "ib/mw/sync/ITimeSyncService.hpp"

#include "IIbToTimeSyncService.hpp"
#include "IParticipantInternal.hpp"
#include "LifecycleService.hpp"
#include "ParticipantConfiguration.hpp"
#include "PerformanceMonitor.hpp"
#include "TimeProvider.hpp"
#include "WatchDog.hpp"

namespace ib {
namespace mw {
namespace sync {

//forward declarations
class SynchronizedVirtualTimeProvider;
class TimeConfiguration;

struct ITimeSyncPolicy;

class TimeSyncService
    : public ITimeSyncService
    , public IIbToTimeSyncService
    , public mw::IIbServiceEndpoint
    , public ITimeProvider
{
    friend struct DistributedTimeQuantumPolicy;

public:
    // ----------------------------------------
    // Public Data Types
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    TimeSyncService(IParticipantInternal* participant, LifecycleService* lifecycleService,
                    const cfg::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // ITimeSyncService
    auto State() const -> ParticipantState;

    void SetSimulationTask(
        std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task) override;
    void SetSimulationTaskAsync(SimTaskT task) override;
    void CompleteSimulationTask() override;

    void SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task) override;

    void SetPeriod(std::chrono::nanoseconds period) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ParticipantCommand& msg) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const NextSimTask& task) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& task) override;

    
    // ITimeProvider
    auto Now() const -> std::chrono::nanoseconds override;
    auto TimeProviderName() const -> const std::string& override;
    auto AddNextSimStepHandler(NextSimStepHandlerT handler) -> HandlerId override;
    void RemoveNextSimStepHandler(HandlerId handlerId) override;
    void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;

    // Used by Policies
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;
    void ExecuteSimTask(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    void InitializeTimeSyncPolicy(bool isSynchronized);
    void ResetTime();
    auto GetTimeProvider() -> std::shared_ptr<sync::ITimeProvider>;
    void ConfigureTimeProvider(sync::TimeProviderKind timeProviderKind);

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

    //void ChangeState(ParticipantState newState, std::string reason);
    void SetPaused(std::future<void> pausedFuture);
    void AwaitNotPaused();
    void ReportError(const std::string& errorMsg);

private:
    // ----------------------------------------
    // private methods
    auto MakeTimeSyncPolicy(bool isSynchronized) -> std::shared_ptr<ITimeSyncPolicy>;

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    LifecycleService* _lifecycleService{nullptr};
    logging::ILogger* _logger{nullptr};
    std::unique_ptr<ITimeProvider> _timeProvider{nullptr};
    std::shared_ptr<TimeConfiguration> _timeConfiguration{nullptr};

    std::shared_ptr<ITimeSyncPolicy> _timeSyncPolicy{nullptr};
    std::vector<std::string> _requiredParticipants;

    bool _isRunning{false};

    bool _isSynchronized;
    bool _timeSyncConfigured{false};

    SimTaskT _simTask;
    std::future<void> _asyncResult;

    util::PerformanceMonitor _execTimeMonitor;
    util::PerformanceMonitor _waitTimeMonitor;
    WatchDog _watchDog;

    // When pausing our participant, message processing is deferred
    // until Continue()'  is called;
    std::promise<void> _pauseDonePromise;
    std::future<void> _pauseDone;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void TimeSyncService::SendIbMessage(MsgT&& msg) const
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}

void TimeSyncService::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto TimeSyncService::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace sync
} // namespace mw
} // namespace ib
