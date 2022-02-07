// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "ib/mw/sync/IParticipantController.hpp"
#include "ib/mw/sync/ITimeProvider.hpp"
#include "ib/cfg/Config.hpp"

#include "PerformanceMonitor.hpp"
#include "WatchDog.hpp"

#include "IIbToParticipantController.hpp"
#include "IComAdapterInternal.hpp"

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
    ParticipantController() = default;
    ParticipantController(IComAdapterInternal* comAdapter, const cfg::SimulationSetup& simulationSetup, const cfg::Participant& participantConfig);
    ParticipantController(const ParticipantController& other) = default;
    ParticipantController(ParticipantController&& other) = default;
    ParticipantController& operator=(const ParticipantController& other) = default;
    ParticipantController& operator=(ParticipantController&& other) = default;

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
    void SetEarliestEventTime(std::chrono::nanoseconds eventTime) override;

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

    // Used by Policies
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;
    void ExecuteSimTask(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);
    void ExecuteSimTaskNonBlocking(std::chrono::nanoseconds timePoint, std::chrono::nanoseconds duration);

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    auto GetTimeProvider()->std::shared_ptr<sync::ITimeProvider>;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;
    
private:
    // ----------------------------------------
    // private methods
    auto MakeTimeSyncPolicy(cfg::SyncType syncType) -> std::unique_ptr<ParticipantController::ITimeSyncPolicy>;
    
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
    IComAdapterInternal* _comAdapter{ nullptr };
    mw::ServiceDescriptor _serviceDescriptor{};
    cfg::SyncType _syncType;
    cfg::TimeSync _timesyncConfig;
    logging::ILogger* _logger{ nullptr };
    std::shared_ptr<ParticipantTimeProvider> _timeProvider{ nullptr };

    std::unique_ptr<ITimeSyncPolicy> _timeSyncPolicy;
    bool _coldswapEnabled{ false };

    service::IServiceDiscovery* _serviceDiscovery;

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

    bool  _waitingForCompletion;
    std::condition_variable  _waitingForCompletionCv;
    std::mutex _waitingForCompletionMutex;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void ParticipantController::SendIbMessage(MsgT&& msg) const
{
    _comAdapter->SendIbMessage(this, std::forward<MsgT>(msg));
}

void ParticipantController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
    // TODO change to isSynchronized for syncType
    if (_syncType == cfg::SyncType::DistributedTimeQuantum)
    {
        _serviceDescriptor.SetSupplementalDataItem(ib::mw::service::controllerIsSynchronized, std::to_string(true));
    }
}

auto ParticipantController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace sync
} // namespace mw
} // namespace ib
