// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "ib/mw/sync/IParticipantController.hpp"
#include "ITimeProvider.hpp"

#include "ParticipantConfiguration.hpp"
#include "PerformanceMonitor.hpp"
#include "WatchDog.hpp"

#include "IIbToParticipantController.hpp"
#include "IParticipantInternal.hpp"

namespace ib {
namespace mw {
namespace sync {

//forward declarations
struct ParticipantTimeProvider;

class ParticipantController
    : public IParticipantController
    , public IIbToParticipantController
    , public mw::IIbServiceEndpoint
{
    friend struct DistributedTimeQuantumPolicy;
public:
    // ----------------------------------------
    // Public Data Types
    struct ITimeSyncPolicy
    {
        virtual ~ITimeSyncPolicy() = default;
        virtual void Initialize() = 0;
        virtual void SynchronizedParticipantAdded(const std::string& otherParticipantName) = 0;
        virtual void SynchronizedParticipantRemoved(const std::string& otherParticipantName) = 0;
        virtual void RequestInitialStep() = 0;
        virtual void RequestNextStep() = 0;
        virtual void SetStepDuration(std::chrono::nanoseconds duration) = 0;
        virtual void ReceiveNextSimTask(const IIbServiceEndpoint* from, const NextSimTask& task) = 0;
        virtual void SetBlockingMode(bool) = 0;
    };

public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    ParticipantController(IParticipantInternal* participant, const std::string& name, bool isSynchronized,
                          const cfg::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // IParticipantController
    void SetInitHandler(InitHandlerT handler) override;
    void SetStopHandler(StopHandlerT handler) override;
    void SetShutdownHandler(ShutdownHandlerT handler) override;
    void SetSimulationTask(std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task) override;
    void SetSimulationTaskAsync(SimTaskT task) override;
    void CompleteSimulationTask() override;

    void SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task) override;

    void EnableColdswap() override;

    void SetPeriod(std::chrono::nanoseconds period) override;

    auto Run()->ParticipantState override;
    auto RunAsync()->std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;
    void Pause(std::string reason) override;
    void Continue() override;
    void Stop(std::string reason) override;
    void ForceShutdown(std::string reason) override;

    auto State() const->ParticipantState override;
    auto Status() const -> const ParticipantStatus & override;
    auto Now() const->std::chrono::nanoseconds override;
    void RefreshStatus() override;

    void LogCurrentPerformanceStats() override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ParticipantCommand& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& msg) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const NextSimTask& task) override;

    // Used to propagate sync participants from monitor
    void AddSynchronizedParticipants(const ExpectedParticipants& participantNames);

    // Used by Policies
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;
    void ExecuteSimTask(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    auto GetTimeProvider()->std::shared_ptr<sync::ITimeProvider>;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;
    
private:
    // ----------------------------------------
    // private methods
    auto MakeTimeSyncPolicy(bool isSynchronized) -> std::unique_ptr<ParticipantController::ITimeSyncPolicy>;
    
    void ChangeState(ParticipantState newState, std::string reason);
    void Initialize(const ParticipantCommand& command, std::string reason);
    void Shutdown(std::string reason);
    void PrepareColdswap();
    void ShutdownForColdswap();
    void IgnoreColdswap();
    
    void AwaitNotPaused();
private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{ nullptr };
    mw::ServiceDescriptor _serviceDescriptor{};
    bool _isSynchronized;
    logging::ILogger* _logger{ nullptr };
    std::shared_ptr<ParticipantTimeProvider> _timeProvider{ nullptr };

    std::unique_ptr<ITimeSyncPolicy> _timeSyncPolicy;

    bool _coldswapEnabled{ false };

    bool _isRunning{ false };
    ParticipantStatus _status;

    std::promise<ParticipantState> _finalStatePromise;

    InitHandlerT _initHandler;
    StopHandlerT _stopHandler;
    ShutdownHandlerT _shutdownHandler;
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
void ParticipantController::SendIbMessage(MsgT&& msg) const
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}

void ParticipantController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto ParticipantController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace sync
} // namespace mw
} // namespace ib
