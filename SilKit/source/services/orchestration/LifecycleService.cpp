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

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/participant/exception.hpp"

#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "LifecycleManagement.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace Orchestration {
LifecycleService::LifecycleService(Core::IParticipantInternal* participant)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    ,_lifecycleManagement{participant->GetLogger(), this}
{
    _timeSyncService = _participant->CreateTimeSyncService(this);

    _status.participantName = _participant->GetParticipantName();
}

 LifecycleService::~LifecycleService()
{
}

void LifecycleService::SetCommunicationReadyHandler(CommunicationReadyHandler handler)
{
    _commReadyHandlerIsAsync = false;
    _commReadyHandler = std::move(handler);
}

void LifecycleService::SetCommunicationReadyHandlerAsync(CommunicationReadyHandler handler)
{
    _commReadyHandlerIsAsync = true;
    _commReadyHandler = std::move(handler);
}

void LifecycleService::CompleteCommunicationReadyHandlerAsync()
{
    _logger->Debug("LifecycleService::CompleteCommunicationReadyHandler: enter");
    // async handler is finished, now continue to the Running state without triggering the CommunicationReadyHandler again
    if(!_commReadyHandlerInvoked)
    {
        _logger->Debug("LifecycleService::CompleteCommunicationReadyHandler: Handler invoked is false, exiting.");
        return;
    }
    if((_lifecycleManagement.GetCurrentState() == _lifecycleManagement.GetCommunicationInitializedState())
        || (_lifecycleManagement.GetCurrentState() == _lifecycleManagement.GetServicesCreatedState())
        )
    {
        _commReadyHandlerInvoked = true;
        _lifecycleManagement.SetState(_lifecycleManagement.GetReadyToRunState(),
            "LifecycleService::CompleteCommunicationReadyHandler: triggering communication ready transition.");
    }
}

void LifecycleService::SetStartingHandler(StartingHandler handler)
{
    _startingHandler = std::move(handler);
}

void LifecycleService::SetStopHandler(StopHandler handler)
{
    _stopHandler = std::move(handler);
}

void LifecycleService::SetShutdownHandler(ShutdownHandler handler)
{
    _shutdownHandler = std::move(handler);
}

auto LifecycleService::StartLifecycle(bool isCoordinated)
    -> std::future<ParticipantState>
{
    if (_timeSyncActive)
    {
        _timeSyncService->ConfigureTimeProvider(TimeProviderKind::SyncTime);
    }
    else
    {
        _timeSyncService->ConfigureTimeProvider(TimeProviderKind::NoSync);
    }
    _timeSyncService->InitializeTimeSyncPolicy(_timeSyncActive);

    _isCoordinated = isCoordinated;

    // Update ServiceDescriptor
    _serviceDescriptor.SetSupplementalDataItem(SilKit::Core::Discovery::lifecycleIsCoordinated,
                                               std::to_string(_isCoordinated));

    // Publish services
    auto serviceDiscovery = _participant->GetServiceDiscovery();
    serviceDiscovery->NotifyServiceCreated(_serviceDescriptor);
    serviceDiscovery->NotifyServiceCreated(_timeSyncService->GetServiceDescriptor());

    _participant->GetSystemMonitor()->AddSystemStateHandler([&](auto systemState) {
        this->NewSystemState(systemState);
    });

    _isRunning = true;
    _lifecycleManagement.InitLifecycleManagement("LifecycleService::StartLifecycle was called.");
    if (!_isCoordinated)
    {
        // Skip state guarantees if start is uncoordinated
        _lifecycleManagement.StartUncoordinated(
            "LifecycleService::StartLifecycle was called without start coordination.");
    }
    return _finalStatePromise.get_future();
}

auto LifecycleService::StartLifecycle(LifecycleConfiguration startConfiguration) -> std::future<ParticipantState>
{
    return StartLifecycle(startConfiguration.isCoordinated);
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

    _lifecycleManagement.Error(std::move(errorMsg));
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
    _lifecycleManagement.Pause(reason);
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

    _lifecycleManagement.Continue("Pause finished");
    _pauseDonePromise.set_value();
}

void LifecycleService::Stop(std::string reason)
{
    _lifecycleManagement.UserStop(reason);
}

void LifecycleService::Shutdown(std::string reason)
{
    auto success = _lifecycleManagement.Shutdown(reason);
    if (success)
    {
        try
        {
            std::stringstream ss;
            ss << "Confirming shutdown of " << _participant->GetParticipantName();
            _logger->Debug(ss.str()); 
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

void LifecycleService::Restart(std::string /*reason*/)
{
    // Currently inoperable
    throw std::runtime_error("Restarting is currently deactivated.");

    //_lifecycleManagement.Restart(reason);
    //
    //if (!_hasCoordinatedSimulationStart)
    //{
    //    _lifecycleManagement->Run("LifecycleService::Restart() was called without start coordination.");
    //}
}

bool LifecycleService::TriggerCommunicationReadyHandler()
{
    if(_commReadyHandler)
    {
        if(_commReadyHandlerIsAsync)
        {
            if(!_commReadyHandlerInvoked)
            {
                _commReadyHandlerInvoked=true;
                _commReadyHandler();
            }
            // Tell the LifecycleManager to not advance to the next state
            // until CompleteCommunicationReadyHandlerAsync is called.
            return false;
        }
        else
        {
            _commReadyHandler();
        }
    }
    return true;
}

void LifecycleService::TriggerStartingHandler()
{
    if (_startingHandler)
    {
        _startingHandler();
    }
}

void LifecycleService::TriggerStopHandler()
{
    if (_stopHandler)
    {
        _stopHandler();
    }
}

void LifecycleService::TriggerShutdownHandler()
{
    if (_shutdownHandler)
    {
        _shutdownHandler();
    }
}

void LifecycleService::AbortSimulation(std::string reason)
{
    _lifecycleManagement.AbortSimulation(reason);
}

auto LifecycleService::State() const -> ParticipantState
{
    return _status.state;
}

auto LifecycleService::Status() const -> const ParticipantStatus&
{
    return _status;
}

auto LifecycleService::GetTimeSyncService() const -> ITimeSyncService*
{
    return _timeSyncService;
}



void LifecycleService::ReceiveMsg(const IServiceEndpoint* from, const SystemCommand& command)
{
    // Ignore messages if the lifecycle is not being executed yet
    if (!_isRunning)
    {
        std::stringstream msg;
        msg << "Received SystemCommand::" << command.kind
            << " before LifecycleService::StartLifecycle(...) was called."
            << " Origin of current command was " << from->GetServiceDescriptor();
        ReportError(msg.str());
        return;
    }

    switch (command.kind)
    {
    case SystemCommand::Kind::Invalid: break;

    case SystemCommand::Kind::AbortSimulation:
        AbortSimulation("Received SystemCommand::AbortSimulation");
        return;
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

    SendMsg(_status);
}

void LifecycleService::SetTimeSyncService(TimeSyncService* timeSyncService)
{
    _timeSyncService = timeSyncService;
}

void LifecycleService::NewSystemState(SystemState systemState)
{
    if (!_isCoordinated)
    {
        // uncoordinated participants do not react to system states changes!
        return;
    }

    std::stringstream ss;
    ss << "Received new system state: " << systemState;

    switch (systemState)
    {
    case SystemState::Invalid: break; // NOP
    case SystemState::ServicesCreated: 
        _lifecycleManagement.SystemwideServicesCreated(ss.str());
        break;
    case SystemState::CommunicationInitializing: break; // ignore
    case SystemState::CommunicationInitialized: 
        _lifecycleManagement.SystemwideCommunicationInitialized(ss.str());
        break;
    case SystemState::ReadyToRun: 
        _lifecycleManagement.SystemwideReadyToRun(ss.str()); 
        break;
    case SystemState::Running: break; // ignore
    case SystemState::Paused: break; // ignore
    case SystemState::Stopping: 
        _lifecycleManagement.SystemwideStopping(ss.str()); 
        break;
    case SystemState::Stopped: break; // To Sync or not to Sync - that is the question -- for now don't
    case SystemState::Error: 
        _lifecycleManagement.Error(ss.str());
        break; // ignore
    case SystemState::ShuttingDown: break; // ignore
    case SystemState::Shutdown: break; // ignore
    }
}

void LifecycleService::SetTimeSyncActive(bool isTimeSyncAcvice)
{
    _timeSyncActive = isTimeSyncAcvice;
}

bool LifecycleService::IsTimeSyncActive() const
{
    return _timeSyncActive;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
