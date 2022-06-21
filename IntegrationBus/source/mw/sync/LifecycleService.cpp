// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "LifecycleManagement.hpp"

#include <cassert>
#include <future>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {
LifecycleService::LifecycleService(IParticipantInternal* participant,
                                   const cfg::HealthCheck& healthCheckConfig)
    : _participant{participant}
    , _logger{participant->GetLogger()}
{
    _timeSyncService = _participant->CreateTimeSyncService(this);

    _status.participantName = _participant->GetParticipantName();
    _lifecycleManagement = std::make_shared<LifecycleManagement>(participant->GetLogger(), this);

    // TODO healthCheckConfig needed?
    (void)healthCheckConfig;
}

void LifecycleService::SetCommunicationReadyHandler(CommunicationReadyHandlerT handler)
{
    _commReadyHandler = std::move(handler);
}

void LifecycleService::SetReinitializeHandler(ReinitializeHandlerT handler)
{
    _reinitializeHandler = std::move(handler);
}

void LifecycleService::SetStopHandler(StopHandlerT handler)
{
    _stopHandler = std::move(handler);
}

void LifecycleService::SetShutdownHandler(ShutdownHandlerT handler)
{
    _shutdownHandler = std::move(handler);
}

auto LifecycleService::ExecuteLifecycle(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                        bool isRequiredParticipant) -> std::future<ParticipantState>
{
    _hasCoordinatedSimulationStart = hasCoordinatedSimulationStart;
    _hasCoordinatedSimulationStop = hasCoordinatedSimulationStop;
    _isRequiredParticipant = isRequiredParticipant;

    // Update ServiceDescriptor
    _serviceDescriptor.SetSupplementalDataItem(ib::mw::service::lifecycleHasCoordinatedStart,
                                               std::to_string(hasCoordinatedSimulationStart));
    _serviceDescriptor.SetSupplementalDataItem(ib::mw::service::lifecycleHasCoordinatedStop,
                                               std::to_string(hasCoordinatedSimulationStop));

    // Publish services
    auto serviceDiscovery = _participant->GetServiceDiscovery();
    serviceDiscovery->NotifyServiceCreated(_serviceDescriptor);
    serviceDiscovery->NotifyServiceCreated(_timeSyncService->GetServiceDescriptor());

    _participant->GetSystemMonitor()->RegisterSystemStateHandler([&](auto systemState) {
        this->NewSystemState(systemState);
    });

    _isRunning = true;
    _lifecycleManagement->InitLifecycleManagement("LifecycleService::ExecuteLifecycle... was called.");
    if (!hasCoordinatedSimulationStart)
    {
        // Skip state guarantees if start is uncoordinated
        _lifecycleManagement->SkipSetupPhase(
            "LifecycleService::ExecuteLifecycle... was called without start coordination.");
    }
    return _finalStatePromise.get_future();
}

auto LifecycleService::ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                bool isRequiredParticipant) -> std::future<ParticipantState>
{
    _timeSyncService->InitializeTimeSyncPolicy(false);

    return ExecuteLifecycle(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, isRequiredParticipant);
}

auto LifecycleService::ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
    -> std::future<ParticipantState>
{
    return ExecuteLifecycleNoSyncTime(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, false);
}

auto LifecycleService::ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService,
                                                    bool hasCoordinatedSimulationStart,
                                                    bool hasCoordinatedSimulationStop, bool isRequiredParticipant)
    -> std::future<ParticipantState>
{
    if (_timeSyncService != timeSyncService)
    {
        throw std::runtime_error("Failed to validate the provided timeSyncService pointer.");
    }

    _timeSyncService->InitializeTimeSyncPolicy(true);

    return ExecuteLifecycle(hasCoordinatedSimulationStart, hasCoordinatedSimulationStop, isRequiredParticipant);
}

auto LifecycleService::ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService,
                                                    bool hasCoordinatedSimulationStart,
                                                    bool hasCoordinatedSimulationStop) -> std::future<ParticipantState>
{
    return ExecuteLifecycleWithSyncTime(timeSyncService, hasCoordinatedSimulationStart, hasCoordinatedSimulationStop,
                                        false);
}

void LifecycleService::ReportError(std::string errorMsg)
{
    _logger->Error(errorMsg);

    // If the lifecycle is not executing, log message and skip further error handling
    if (State() == ParticipantState::Invalid)
    {
        return;
    }

    if (State() == ParticipantState::Shutdown)
    {
        _logger->Warn("LifecycleService::ReportError() was called in terminal state ParticipantState::Shutdown; "
                      "transition to ParticipantState::Error is ignored.");
        return;
    }

    _lifecycleManagement->Error(std::move(errorMsg));
}

void LifecycleService::Pause(std::string reason)
{
    if (State() != ParticipantState::Running)
    {
        const std::string errorMessage{"TimeSyncService::Pause() was called in state ParticipantState::"
                                       + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }
    _pauseDonePromise = decltype(_pauseDonePromise){};
    _timeSyncService->SetPaused(_pauseDonePromise.get_future());
    _lifecycleManagement->Pause(reason);
}

void LifecycleService::Continue()
{
    if (State() != ParticipantState::Paused)
    {
        const std::string errorMessage{"TimeSyncService::Continue() was called in state ParticipantState::"
                                       + to_string(State())};
        ReportError(errorMessage);
        throw std::runtime_error(errorMessage);
    }

    _lifecycleManagement->Continue("Pause finished");
    _pauseDonePromise.set_value();
}

void LifecycleService::Stop(std::string reason)
{
    _lifecycleManagement->Stop(reason);

    if (!_hasCoordinatedSimulationStop) 
    {
        Shutdown("Shutdown after LifecycleService::Stop without stop coordination.");
    }
}

void LifecycleService::TriggerStopHandler(std::string)
{
    if (_stopHandler)
    {
        _stopHandler();
    }
}

void LifecycleService::Shutdown(std::string reason)
{
    auto success = _lifecycleManagement->Shutdown(reason);
    if (success)
    {
        try
        {
            _finalStatePromise.set_value(State());
        }
        catch (const std::future_error&)
        {
            // NOP - received shutdown multiple times
        }
    }
    else
    {
        _logger->Warn("lifecycle failed to shut down correctly - original shutdown reason was '{}'.",
                      std::move(reason));
    }
}

void LifecycleService::TriggerShutdownHandler(std::string)
{
    if (_shutdownHandler)
    {
        _shutdownHandler();
    }
}


void LifecycleService::Reinitialize(std::string reason)
{
    _lifecycleManagement->Reinitialize(reason);

    if (!_hasCoordinatedSimulationStart)
    {
        _lifecycleManagement->Run("LifecycleService::Reinitialize() was called without start coordination.");
    }
}

void LifecycleService::TriggerCommunicationReadyHandler(std::string)
{
    if (_commReadyHandler)
    {
        _commReadyHandler();
    }
}

void LifecycleService::TriggerReinitializeHandle(std::string)
{
    if (_reinitializeHandler)
    {
        _reinitializeHandler();
        _timeSyncService->ResetTime();
    }
}

void LifecycleService::AbortSimulation(std::string reason)
{
    auto success = _lifecycleManagement->AbortSimulation(reason);
    if (success)
    {
        try
        {
            _finalStatePromise.set_value(State());
        }
        catch (const std::future_error&)
        {
            // NOP - received shutdown multiple times
        }
    }
}

auto LifecycleService::State() const -> ParticipantState
{
    return _status.state;
}

auto LifecycleService::Status() const -> const ParticipantStatus&
{
    return _status;
}

void LifecycleService::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const ParticipantCommand& command)
{
    if (command.participant != _serviceDescriptor.GetParticipantId())
        return;
    
    if (_hasCoordinatedSimulationStop)
    {
        if (command.kind == ParticipantCommand::Kind::Reinitialize)
        {
            Reinitialize(std::string{"Received ParticipantCommand::"} + to_string(command.kind));
        }
        else if (command.kind == ParticipantCommand::Kind::Shutdown)
        {
            Shutdown("Received ParticipantCommand::Shutdown");
        }
    }
}

auto LifecycleService::GetTimeSyncService() const -> ITimeSyncService*
{
    return _timeSyncService;
}

void LifecycleService::ReceiveIbMessage(const IIbServiceEndpoint* from, const SystemCommand& command)
{
    // Ignore messages if the lifecycle is not being executed yet
    if (!_isRunning)
    {
        // TODO this should be handled as a late joining scenario instead of an error...
        std::stringstream msg;
        msg << "Received SystemCommand::" << command.kind
            << " before LifecycleService::ExecuteLifecycleSyncTime(...) or ExecuteLifecycleNoSyncTime(...) was called."
            << " Origin of current command was " << from->GetServiceDescriptor();
        ReportError(msg.str());
        return;
    }

    switch (command.kind)
    {
    case SystemCommand::Kind::Invalid: break;

    case SystemCommand::Kind::Run:
        if (!_hasCoordinatedSimulationStart)
        {
            _logger->Info(
                "Received SystemCommand::Start, but ignored it because coordinatedSimulationStart was not set.");
            return;
        }
        else
        {
            _lifecycleManagement->Run("Received SystemCommand::Run");
            return;
        }
        break;

    case SystemCommand::Kind::Stop:

        if (!_hasCoordinatedSimulationStop)
        {
            _logger->Info(
                "Received SystemCommand::Stop, but ignored it because coordinatedSimulationStop was not set.");
            return;
        }
        Stop("Received SystemCommand::Stop");
        return;

    case SystemCommand::Kind::Shutdown:
        Shutdown("Received SystemCommand::Shutdown");
        return;

    case SystemCommand::Kind::AbortSimulation:
        AbortSimulation("Received SystemCommand::AbortSimulation");
        return;

    // TODO Remove legacy code (VIB-790)
    case SystemCommand::Kind::PrepareColdswap: 
    case SystemCommand::Kind::ExecuteColdswap: 
    default: break;
    }

    // We should not reach this point in normal operation.
    ReportError("Received SystemCommand::" + to_string(command.kind)
                + " while in ParticipantState::" + to_string(State()));
}

void LifecycleService::ChangeState(ParticipantState newState, std::string reason)
{
    _status.state = newState;
    _status.enterReason = std::move(reason);
    _status.enterTime = std::chrono::system_clock::now();
    _status.refreshTime = _status.enterTime;

    std::stringstream ss;
    ss << "New ParticipantState: " << newState << "; reason: " << _status.enterReason;
    _logger->Info(ss.str());

    SendIbMessage(_status);
}

void LifecycleService::SetTimeSyncService(TimeSyncService* timeSyncService)
{
    _timeSyncService = timeSyncService;
}

void LifecycleService::NewSystemState(SystemState systemState)
{
    _lifecycleManagement->NewSystemState(systemState);
}

} // namespace sync
} // namespace mw
} // namespace ib
