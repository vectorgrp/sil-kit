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

#include "silkit/services/orchestration/ISystemMonitor.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/participant/exception.hpp"

#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"
#include "IServiceDiscovery.hpp"
#include "ILoggerInternal.hpp"
#include "LifecycleManagement.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace Orchestration {

LifecycleService::LifecycleService(Core::IParticipantInternal* participant)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    , _lifecycleManager{participant, participant->GetLogger(), this}
    , _finalStatePromise{std::make_unique<std::promise<ParticipantState>>()}
    , _finalStateFuture{_finalStatePromise->get_future()}
{
    _timeSyncService = _participant->CreateTimeSyncService(this);
}

LifecycleService::~LifecycleService() {}

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
    if (!_commReadyHandlerInvoked)
    {
        _logger->Debug("LifecycleService::CompleteCommunicationReadyHandler was called without invoking the "
                       "CommunicationReadyHandler, ignoring.");
        return;
    }
    if (_commReadyHandlerCompleted)
    {
        _logger->Debug("LifecycleService::CompleteCommunicationReadyHandler has been called already, ignoring.");
        return;
    }
    _commReadyHandlerCompleted = true;

    _participant->ExecuteDeferred([this] {
        _lifecycleManager.CompleteCommunicationReadyHandler(
            "LifecycleService::CompleteCommunicationReadyHandler: triggering communication ready transition.");
    });
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

void LifecycleService::SetAbortHandler(AbortHandler handler)
{
    _abortHandler = std::move(handler);
}

auto LifecycleService::StartLifecycle() -> std::future<ParticipantState>
{
    std::stringstream ss;
    ss << "Lifecycle of participant " << _participant->GetParticipantName() << " started";
    _logger->Debug(ss.str());

    _isLifecycleStarted = true;

    if (_finalStatePromise == nullptr)
    {
        throw LogicError{"LifecycleService::StartLifecycle must not be called twice"};
    }

    if (_abortedBeforeLifecycleStart)
    {
        _participant->ExecuteDeferred([this] {
            _logger->Warn(
                "LifecycleService::StartLifecycle(...) was called after receiving SystemCommand::AbortSimulation;");
            _lifecycleManager.AbortSimulation("Lifecycle was aborted by SystemCommand::AbortSimulation");
        });
        return std::move(_finalStateFuture);
    }

    if (HasRequiredParticipantNames())
    {
        if (!CheckForValidConfiguration())
        {
            return std::move(_finalStateFuture);
        }
    }
    if (_timeSyncActive)
    {
        _timeSyncService->ConfigureTimeProvider(TimeProviderKind::SyncTime);
    }
    else
    {
        _timeSyncService->ConfigureTimeProvider(TimeProviderKind::NoSync);
    }
    _timeSyncService->InitializeTimeSyncPolicy(_timeSyncActive);

    // Update ServiceDescriptor
    _serviceDescriptor.SetSupplementalDataItem(SilKit::Core::Discovery::lifecycleIsCoordinated,
                                               std::to_string(static_cast<int>(_operationMode.load())));

    // Publish services
    auto serviceDiscovery = _participant->GetServiceDiscovery();
    serviceDiscovery->NotifyServiceCreated(_serviceDescriptor);
    serviceDiscovery->NotifyServiceCreated(_timeSyncService->GetServiceDescriptor());

    _participant->GetSystemMonitor()->AddSystemStateHandler(
        [&](auto systemState) { this->NewSystemState(systemState); });

    _participant->GetSystemMonitor()->AddParticipantStatusHandler(
        [&](auto participantStatus) { this->NewParticipantStatus(participantStatus); });

    // Initialize switches to ServicesCreated. In parallel, the reception of an invalid WorkflowConfiguration could
    // lead to ErrorState, so we need to defer this to the IO-Worker Thread
    _participant->ExecuteDeferred(
        [this] { _lifecycleManager.Initialize("LifecycleService::StartLifecycle was called."); });

    switch (_operationMode)
    {
    case OperationMode::Invalid:
        throw ConfigurationError("OperationMode was not set. This is mandatory.");
    case OperationMode::Coordinated:
        // ServicesCreated is triggered by SystemState
        break;
    case OperationMode::Autonomous:
        // Autonomous start direcly, coordinated wait for SystemState Change
        _participant->ExecuteDeferred([this] {
            _lifecycleManager.StartAutonomous(
                "LifecycleService::StartLifecycle was called without start coordination.");
        });

        break;
    }
    return std::move(_finalStateFuture);
}

void LifecycleService::ReportError(std::string errorMsg)
{
    _participant->ExecuteDeferred([errorMsg, this] {
        _logger->Error(errorMsg);

        if (!_isLifecycleStarted)
        {
            _logger->Warn(
                "LifecycleService::ReportError() was called before LifecycleService::StartLifecycle() was called;"
                "transition to ParticipantState::Error is ignored.");
            return;
        }
        else if (State() == ParticipantState::Shutdown)
        {
            _logger->Warn("LifecycleService::ReportError() was called in terminal state ParticipantState::Shutdown; "
                          "transition to ParticipantState::Error is ignored.");
            return;
        }

        _lifecycleManager.Error(std::move(errorMsg));
    });
}

void LifecycleService::Pause(std::string reason)
{
    _pauseRequested = true;
    _participant->ExecuteDeferred([this, reason] {
        if (State() != ParticipantState::Running)
        {
            const std::string errorMessage{"LifecycleService::Pause() was called in state ParticipantState::"
                                           + to_string(State())};
            ReportError(errorMessage);
            throw SilKitError(errorMessage);
        }
        _lifecycleManager.Pause(reason);
    });
}

void LifecycleService::Continue()
{
    _pauseRequested = false;
    _participant->ExecuteDeferred([this] {
        if (State() != ParticipantState::Paused)
        {
            const std::string errorMessage{"LifecycleService::Continue() was called in state ParticipantState::"
                                           + to_string(State())};
            ReportError(errorMessage);
            throw SilKitError(errorMessage);
        }

        _lifecycleManager.Continue("Pause finished");
        _timeSyncService->RequestNextStep();
    });
}

void LifecycleService::Stop(std::string reason)
{
    _stopRequested = true;
    _participant->ExecuteDeferred([this, reason] { _lifecycleManager.Stop(reason); });
}

void LifecycleService::Restart(std::string /*reason*/)
{
    throw SilKitError("Restarting is currently not supported.");
}

void LifecycleService::SetLifecycleConfiguration(LifecycleConfiguration startConfiguration)
{
    _operationMode = startConfiguration.operationMode;
}

OperationMode LifecycleService::GetOperationMode() const
{
    return _operationMode;
}

bool LifecycleService::TriggerCommunicationReadyHandler()
{
    if (_commReadyHandler)
    {
        if (_commReadyHandlerIsAsync)
        {
            if (!_commReadyHandlerInvoked)
            {
                _commReadyHandlerInvoked = true;
                _commReadyHandlerCompleted = false;
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

void LifecycleService::TriggerAbortHandler(ParticipantState lastState)
{
    if (_abortHandler)
    {
        _abortHandler(lastState);
    }
}

void LifecycleService::AbortSimulation(std::string reason)
{
    // Ignore messages if StartLifecycle has not been called yet
    if (!_isLifecycleStarted)
    {
        std::stringstream msg;
        msg << "Received SystemCommand::AbortSimulation before LifecycleService::StartLifecycle(...) was called.";
        _logger->Warn(msg.str());
        // If StartLifecycle is called afterwards, this flag is checked to trigger the actual abort handling.
        _abortedBeforeLifecycleStart = true;
        return;
    }
    _participant->ExecuteDeferred([this, reason] { _lifecycleManager.AbortSimulation(reason); });
}

bool LifecycleService::CheckForValidConfiguration()
{
    if (_operationMode == OperationMode::Coordinated)
    {
        const auto isNotRequiredParticipant = [this]() -> bool {
            std::unique_lock<decltype(_requiredParticipantNamesMx)> lock{_requiredParticipantNamesMx};
            auto it = std::find(_requiredParticipantNames.begin(), _requiredParticipantNames.end(),
                                _participant->GetParticipantName());
            return it == _requiredParticipantNames.end();
        }();

        if (isNotRequiredParticipant)
        {
            // participants that are coordinated but not required are currently not supported
            std::stringstream ss;

            ss << _participant->GetParticipantName()
               << ": This participant is in OperationMode::Coordinated but it is not part of the "
                  "set of \"required\" participants declared by the system controller. ";

            ReportError(ss.str());

            return false;
        }
    }
    return true;
}

void LifecycleService::SetFinalStatePromise()
{
    try
    {
        _logger->Debug("Setting the final state promise");
        _finalStatePromise->set_value(State());
    }
    catch (const std::future_error&)
    {
        // NOP - received shutdown multiple times
    }
}

auto LifecycleService::State() const -> ParticipantState
{
    std::unique_lock<decltype(_statusMx)> lock{_statusMx};
    return _status.state;
}

auto LifecycleService::StopRequested() const -> bool
{
    return _stopRequested;
}
auto LifecycleService::PauseRequested() const -> bool
{
    return _pauseRequested;
}

auto LifecycleService::Status() const -> const ParticipantStatus&
{
    // NB: This method cannot return a reference to the 'actual' current status, since that object is modified
    //     concurrently.
    //
    //     Using the returned reference from this method is only safe as long as it is not also called concurrently.
    //     Using the returned reference is also only valid until the next call to this method.
    //
    //     Calling this method concurrently on multiple threads, and using any member of the return value will result
    //     not only in data-races, but can also cause access to already freed memory and may result in a crash.
    //     The same issues arise when using a reference returned by a previous call to this method.

    // copy the current status under lock
    {
        std::unique_lock<decltype(_statusMx)> lock{_statusMx};
        _returnValueForStatus = _status;
    }

    // return a reference to the copy
    return _returnValueForStatus;
}

void LifecycleService::AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    _participant->AddAsyncSubscriptionsCompletionHandler(std::move(handler));
}

auto LifecycleService::GetTimeSyncService() -> ITimeSyncService*
{
    return _timeSyncService;
}

auto LifecycleService::CreateTimeSyncService() -> ITimeSyncService*
{
    if (!_timeSyncActive)
    {
        _participant->RegisterTimeSyncService(_timeSyncService);
        _timeSyncActive = true;
        return _timeSyncService;
    }
    else
    {
        throw ConfigurationError("You may not create the time synchronization service more than once.");
    }
}

void LifecycleService::ReceiveMsg(const IServiceEndpoint*, const SystemCommand& command)
{
    switch (command.kind)
    {
    case SystemCommand::Kind::Invalid:
        break;

    case SystemCommand::Kind::AbortSimulation:
        AbortSimulation("Received SystemCommand::AbortSimulation");
        return;
    default:
        break;
    }

    // We should not reach this point in normal operation.
    ReportError("Received SystemCommand::" + to_string(command.kind)
                + " while in ParticipantState::" + to_string(State()));
}

void LifecycleService::SetWorkflowConfiguration(const WorkflowConfiguration& configuration)
{
    SetRequiredParticipantNames(configuration.requiredParticipantNames);
    // Receiving the WorkflowConfiguration after lifecycle start
    if (_operationMode != OperationMode::Invalid && _isLifecycleStarted)
    {
        CheckForValidConfiguration();
    }
}

void LifecycleService::ChangeParticipantState(ParticipantState newState, std::string reason)
{
    ParticipantStatus status{};
    status.participantName = _participant->GetParticipantName();
    status.state = newState;
    status.enterReason = std::move(reason);
    status.enterTime = std::chrono::system_clock::now();
    status.refreshTime = _status.enterTime;

    std::stringstream ss;
    ss << "New ParticipantState: " << newState << "; reason: " << status.enterReason;
    _logger->Debug(ss.str());

    // assign the current status under lock (copy)
    {
        std::unique_lock<decltype(_statusMx)> lock{_statusMx};
        _status = status;
    }

    SendMsg(status);
}

void LifecycleService::SetTimeSyncService(TimeSyncService* timeSyncService)
{
    _timeSyncService = timeSyncService;
}

void LifecycleService::NewSystemState(SystemState systemState)
{
    switch (_operationMode)
    {
    case OperationMode::Invalid:
        // ignore
        return;
    case OperationMode::Coordinated:
        break;
    case OperationMode::Autonomous:
        // autonomous participants do not react to system states changes!
        return;
    }

    std::stringstream ss;
    ss << "Received new system state: " << systemState;
    Logging::Debug(_logger, ss.str().c_str());

    switch (systemState)
    {
    case SystemState::Invalid:
        break;
    case SystemState::ServicesCreated:
        _lifecycleManager.ServicesCreated(ss.str());
        break;
    case SystemState::CommunicationInitializing:
        break;
    case SystemState::CommunicationInitialized:
        _lifecycleManager.CommunicationInitialized(ss.str());
        break;
    case SystemState::ReadyToRun:
        _lifecycleManager.ReadyToRun(ss.str());
        break;
    case SystemState::Running:
        _logger->Info("Simulation is now running");
        break;
    case SystemState::Paused:
        _logger->Info("Simulation is paused");
        break;
    case SystemState::Stopping:
        _logger->Info("Simulation is stopping");
        // Only allow external stop signal if we are actually running or paused
        if (_lifecycleManager.GetCurrentState() == _lifecycleManager.GetRunningState()
            || _lifecycleManager.GetCurrentState() == _lifecycleManager.GetPausedState())
        {
            _lifecycleManager.Stop(ss.str());
        }
        break;
    case SystemState::Stopped:
        break;
    case SystemState::ShuttingDown:
        break;
    case SystemState::Shutdown:
        _logger->Info("Simulation is shut down");
        break;
    case SystemState::Aborting:
        break;
    case SystemState::Error:
        break;
    }
}

void LifecycleService::NewParticipantStatus(const ParticipantStatus& participantStatus)
{
    if (participantStatus.participantName == _participant->GetParticipantName())
    {
        if (participantStatus.state == ParticipantState::ReadyToRun)
        {
            if (IsTimeSyncActive())
            {
                _participant->EvaluateAggregationInfo(_timeSyncService->IsBlocking());
            }
        }
    }
}

void LifecycleService::SetTimeSyncActive(bool isTimeSyncActive)
{
    _timeSyncActive = isTimeSyncActive;
}

bool LifecycleService::IsTimeSyncActive() const
{
    return _timeSyncActive;
}

void LifecycleService::SetRequiredParticipantNames(const std::vector<std::string>& requiredParticipantNames)
{
    std::unique_lock<decltype(_requiredParticipantNamesMx)> lock{_requiredParticipantNamesMx};
    _requiredParticipantNames = requiredParticipantNames;
}

bool LifecycleService::HasRequiredParticipantNames() const
{
    std::unique_lock<decltype(_requiredParticipantNamesMx)> lock{_requiredParticipantNamesMx};
    return !_requiredParticipantNames.empty();
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
