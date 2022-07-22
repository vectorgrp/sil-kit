// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "silkit/services/orchestration/ITimeSyncService.hpp"

#include "IMsgForTimeSyncService.hpp"
#include "IParticipantInternal.hpp"
#include "LifecycleService.hpp"
#include "ParticipantConfiguration.hpp"
#include "PerformanceMonitor.hpp"
#include "TimeProvider.hpp"
#include "TimeConfiguration.hpp"
#include "WatchDog.hpp"

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
    friend struct DistributedTimeQuantumPolicy;

public:
    // ----------------------------------------
    // Public Data Types
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    TimeSyncService(Core::IParticipantInternal* participant, ITimeProvider* timeProvider,
                    const Config::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // ITimeSyncService
    auto State() const -> ParticipantState;

    void SetSimulationStepHandler(
        std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task,
        std::chrono::nanoseconds initialStepSize) override;
    void SetSimulationStepHandlerAsync(SimulationStepT task, std::chrono::nanoseconds initialStepSize) override;
    void CompleteSimulationStep() override;

    void SetSimulationStepHandler(std::function<void(std::chrono::nanoseconds now)> task,
                                  std::chrono::nanoseconds initialStepSize) override;

    void SetPeriod(std::chrono::nanoseconds period);

    void ReceiveMsg(const IServiceEndpoint* from, const ParticipantCommand& msg) override;

    void ReceiveMsg(const IServiceEndpoint* from, const NextSimTask& task) override;

    void ReceiveMsg(const IServiceEndpoint* from, const SystemCommand& task) override;

    auto Now() const -> std::chrono::nanoseconds override;

    // Used by Policies
    template <class MsgT>
    void SendMsg(MsgT&& msg) const;
    void ExecuteSimStep(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    void InitializeTimeSyncPolicy(bool isSynchronized);
    void ResetTime();
    void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind);

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    //void ChangeState(ParticipantState newState, std::string reason);
    void SetPaused(std::future<void> pausedFuture);
    void AwaitNotPaused();
    void ReportError(const std::string& errorMsg);

    bool IsSynchronized();

    void SetLifecycleService(LifecycleService* lifecycleService);

private:
    // ----------------------------------------
    // private methods
    auto MakeTimeSyncPolicy(bool isSynchronized) -> std::shared_ptr<ITimeSyncPolicy>;

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    LifecycleService* _lifecycleService{nullptr};
    Services::Logging::ILogger* _logger{nullptr};
    ITimeProvider* _timeProvider{nullptr};
    TimeConfiguration _timeConfiguration;

    std::shared_ptr<ITimeSyncPolicy> _timeSyncPolicy{nullptr};
    std::vector<std::string> _requiredParticipants;

    bool _isRunning{false};
    bool _isSynchronized{false};
    bool _timeSyncConfigured{false};

    SimulationStepT _simTask;
    std::future<void> _asyncResult;

    Util::PerformanceMonitor _execTimeMonitor;
    Util::PerformanceMonitor _waitTimeMonitor;
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

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
