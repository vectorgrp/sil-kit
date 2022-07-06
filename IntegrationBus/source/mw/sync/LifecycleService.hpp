// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "silkit/core/sync/ILifecycleService.hpp"

#include "PerformanceMonitor.hpp"
#include "WatchDog.hpp"

#include "IMsgForLifecycleService.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

//forward declarations
class SynchronizedVirtualTimeProvider;
class TimeSyncService;
class ILifecycleManagement;
struct LifecycleConfiguration;

class LifecycleService
    : public ILifecycleService
    , public IMsgForLifecycleService
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    LifecycleService(IParticipantInternal* participant,
                     const Config::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // ILifecycleService
    void SetCommunicationReadyHandler(CommunicationReadyHandlerT handler) override;
    void SetStartingHandler(StartingHandlerT handler) override;
    void SetStopHandler(StopHandlerT handler) override;
    void SetShutdownHandler(ShutdownHandlerT handler) override;

    auto GetTimeSyncService() const -> ITimeSyncService* override;

    auto StartLifecycleNoSyncTime(LifecycleConfiguration startConfiguration)
        -> std::future<ParticipantState> override;
    auto StartLifecycleWithSyncTime(ITimeSyncService* timeSyncService,
            LifecycleConfiguration startConfiguration)
        -> std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;

    void Pause(std::string reason) override;
    void Continue() override;

    void Stop(std::string reason) override;

    auto State() const -> ParticipantState override;
    auto Status() const -> const ParticipantStatus& override;

    void ReceiveSilKitMessage(const IServiceEndpoint* from, const ParticipantCommand& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const SystemCommand& msg) override;

    // Used by Policies
    template <class MsgT>
    void SendMsg(MsgT&& msg) const;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;


public:
    void TriggerCommunicationReadyHandler(std::string reason);
    void TriggerStartingHandler(std::string reason);
    void TriggerStopHandler(std::string reason);
    void TriggerShutdownHandler(std::string reason);

    void ChangeState(ParticipantState newState, std::string reason);

    void SetTimeSyncService(TimeSyncService* timeSyncService);

    void NewSystemState(SystemState systemState);

    bool IsTimeSyncActive();


private:
    // ----------------------------------------
    // private methods
    auto StartLifecycle(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
        -> std::future<ParticipantState>;

    void Shutdown(std::string reason);
    void Restart(std::string reason);
    void AbortSimulation(std::string reason);

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    Logging::ILogger* _logger{nullptr};

    TimeSyncService* _timeSyncService;

    bool _hasCoordinatedSimulationStart = false;
    bool _hasCoordinatedSimulationStop = false;

    bool _isRunning{false};
    ParticipantStatus _status;
    std::shared_ptr<ILifecycleManagement> _lifecycleManagement;
    bool _timeSyncActive = false;

    std::promise<ParticipantState> _finalStatePromise;

    CommunicationReadyHandlerT _commReadyHandler;
    StartingHandlerT _startingHandler;
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
void LifecycleService::SendMsg(MsgT&& msg) const
{
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}

void LifecycleService::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LifecycleService::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
