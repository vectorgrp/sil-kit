// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "ib/mw/sync/ILifecycleService.hpp"

#include "PerformanceMonitor.hpp"
#include "WatchDog.hpp"

#include "IIbToLifecycleService.hpp"
#include "IParticipantInternal.hpp"


namespace ib {
namespace mw {
namespace sync {

//forward declarations
struct ParticipantTimeProvider;
class TimeSyncService;
class ILifecycleManagement;

class LifecycleService
    : public ILifecycleService
    , public IIbToLifecycleService
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    LifecycleService(IParticipantInternal* participant,
                     const cfg::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // ILifecycleService
    void SetCommunicationReadyHandler(CommunicationReadyHandlerT handler) override;
    void SetReinitializeHandler(ReinitializeHandlerT handler) override;
    void SetStopHandler(StopHandlerT handler) override;
    void SetShutdownHandler(ShutdownHandlerT handler) override;

    auto GetTimeSyncService() const -> ITimeSyncService* override;

    auto ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                    bool isRequiredParticipant) -> std::future<ParticipantState> override;
    auto ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
        -> std::future<ParticipantState> override;
    auto ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService, bool hasCoordinatedSimulationStart,
                                      bool hasCoordinatedSimulationStop, bool isRequiredParticipant)
        -> std::future<ParticipantState> override;
    auto ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService, bool hasCoordinatedSimulationStart,
                                      bool hasCoordinatedSimulationStop) -> std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;

    void Pause(std::string reason) override;
    void Continue() override;

    void Stop(std::string reason) override;

    auto State() const -> ParticipantState override;
    auto Status() const -> const ParticipantStatus& override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const ParticipantCommand& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& msg) override;

    // Used by Policies
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;


public:
    void TriggerCommunicationReadyHandle(std::string reason);
    void TriggerReinitializeHandle(std::string reason);
    void TriggerStopHandle(std::string reason);
    void TriggerShutdownHandle(std::string reason);

    void ChangeState(ParticipantState newState, std::string reason);

    void SetTimeSyncService(TimeSyncService* timeSyncService);

    void NewSystemState(SystemState systemState);


private:
    // ----------------------------------------
    // private methods
    auto ExecuteLifecycle(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                          bool isRequiredParticipant) -> std::future<ParticipantState>;

    void Shutdown(std::string reason);
    void Reinitialize(std::string reason);
    void AbortSimulation(std::string reason);

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    logging::ILogger* _logger{nullptr};

    TimeSyncService* _timeSyncService;

    bool _hasCoordinatedSimulationStart = false;
    bool _hasCoordinatedSimulationStop = false;
    bool _isRequiredParticipant = false;

    bool _isRunning{false};
    ParticipantStatus _status;
    std::shared_ptr<ILifecycleManagement> _lifecycleManagement;

    std::promise<ParticipantState> _finalStatePromise;

    CommunicationReadyHandlerT _commReadyHandler;
    ReinitializeHandlerT _reinitializeHandler;
    StopHandlerT _stopHandler;
    ShutdownHandlerT _shutdownHandler;
    std::future<void> _asyncResult;

    // When pausing our participant, message processing is deferred
    // until Continue()  is called;
    std::promise<void> _pauseDonePromise;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void LifecycleService::SendIbMessage(MsgT&& msg) const
{
    _participant->SendIbMessage(this, std::forward<MsgT>(msg));
}

void LifecycleService::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LifecycleService::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace sync
} // namespace mw
} // namespace ib
