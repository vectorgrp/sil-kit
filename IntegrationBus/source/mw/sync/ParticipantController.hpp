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
public:
    // ----------------------------------------
    // Public Data Types
    struct ISyncAdapter
    {
        virtual ~ISyncAdapter() = default;
        virtual void RequestStep(ParticipantController& controller) = 0;
        virtual void FinishedStep(ParticipantController& controller) = 0;
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

    // IIbToParticipantController
    void SetEndpointAddress(const mw::EndpointAddress& addr) override;
    auto EndpointAddress() const -> const mw::EndpointAddress & override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ParticipantCommand& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& msg) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const Tick& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const QuantumGrant& msg) override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const NextSimTask& task) override;

    // Used by Policies
    void SendTickDone() const;
    void SendQuantumRequest() const;
    void SendNextSimTask();
    void ExecuteSimTask();

    // Get the instance of the internal ITimeProvider that is updated with our simulation time
    auto GetTimeProvider()->std::shared_ptr<sync::ITimeProvider>;

    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;
private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;

    static auto MakeSyncAdapter(ib::cfg::SyncType syncType)->std::unique_ptr<ParticipantController::ISyncAdapter>;

    void ChangeState(ParticipantState newState, std::string reason);

    void Initialize(const ParticipantCommand& command, std::string reason);
    void Shutdown(std::string reason);
    void PrepareColdswap();
    void ShutdownForColdswap();
    void IgnoreColdswap();
    void ProcessQuantumGrant(const QuantumGrant& msg);
    void CheckDistributedTimeAdvanceGrant();

private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{ nullptr };
    mw::ServiceId _serviceId{};
    cfg::SyncType _syncType;
    cfg::TimeSync _timesyncConfig;
    logging::ILogger* _logger{ nullptr };
    std::shared_ptr<ParticipantTimeProvider> _timeProvider{ nullptr };

    std::unique_ptr<ISyncAdapter> _syncAdapter;
    bool _coldswapEnabled{ false };

    ParticipantStatus _status;

    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<ParticipantId, NextSimTask> _otherNextTasks;

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
    _comAdapter->SendIbMessage(this, std::forward<MsgT>(msg));
}

void ParticipantController::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto ParticipantController::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

    
} // namespace sync
} // namespace mw
} // namespace ib
