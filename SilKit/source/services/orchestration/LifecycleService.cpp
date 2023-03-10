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
#include "ILogger.hpp"
#include "LifecycleManagement.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace Orchestration {

LifecycleService::LifecycleService(Core::IParticipantInternal* participant,
                                   std::chrono::milliseconds systemStateReachedShuttingDownTimeout)
    : _participant{participant}
    , _logger{participant->GetLogger()}
    , _lifecycleManagement{participant->GetLogger(), this}
    , _finalStatePromise{std::make_unique<std::promise<ParticipantState>>()}
    , _finalStateFuture{_finalStatePromise->get_future()}
    , _systemStateReachedShuttingDownTimeout{systemStateReachedShuttingDownTimeout}
{
    _timeSyncService = _participant->CreateTimeSyncService(this);
}

 LifecycleService::~LifecycleService()
{
     if (_finalStatePromiseSetterThread.joinable())
     {
         _finalStatePromiseSetterThread.join();
     }
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

void LifecycleService::SetAbortHandler(AbortHandler handler)
{
    _abortHandler = std::move(handler);
}

auto LifecycleService::StartLifecycle()
    -> std::future<ParticipantState>
{
    if (_finalStatePromise == nullptr || _finalStatePromiseSetterThread.joinable())
    {
        throw LogicError{"LifecycleService::StartLifecycle must not be called twice"};
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

    _participant->GetSystemMonitor()->AddSystemStateHandler([&](auto systemState) {
        this->NewSystemState(systemState);
    });

    _isRunning = true;
    _lifecycleManagement.InitLifecycleManagement("LifecycleService::StartLifecycle was called.");
    switch (_operationMode)
    {
    case OperationMode::Invalid: 
        throw ConfigurationError("OperationMode was not set. This is mandatory.");
    case OperationMode::Coordinated: 
        break;
    case OperationMode::Autonomous:
        // Skip state guarantees if start is autonomous
        _lifecycleManagement.StartUncoordinated(
            "LifecycleService::StartLifecycle was called without start coordination.");
        break;
    }
    return std::move(_finalStateFuture);
}

void LifecycleService::ReportError(std::string errorMsg)
{
    _logger->Error(errorMsg);

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
        throw SilKitError(errorMessage);
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
        throw SilKitError(errorMessage);
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

            // The following code is a workaround which alleviates the potential loss of the 'stop' signal issued by
            // the lifecycle service of (e.g.) the system-controller.
            //
            // Setting the final-state promise is delayed up to a timeout value.

            // steal the final state promise pointer in a thread-safe way (protected by a mutex)
            auto finalStatePromise = [this] {
                const auto lock = std::unique_lock<decltype(_finalStatePromiseMutex)>{_finalStatePromiseMutex};
                return std::move(_finalStatePromise);
            }();

            // if we stole the actual promise object, we start the thread which actually sets it
            if (finalStatePromise)
            {
                _finalStatePromiseSetterThread = std::thread{
                    [logger = _logger, timeout = _systemStateReachedShuttingDownTimeout,
                     systemStateReachedShuttingDownFuture = _systemStateReachedShuttingDownPromise.get_future(),
                     finalStatePromise = std::move(finalStatePromise), state = State()] {
                        try
                        {
                            systemStateReachedShuttingDownFuture.wait_for(timeout);
                        }
                        catch (const std::exception& exception)
                        {
                            Logging::Warn(logger, "LifecycleService: Waiting for the system state failed: {}",
                                          exception.what());
                        }
                        catch (...)
                        {
                            Logging::Warn(logger, "LifecycleService: Waiting for the system state failed: unknown");
                        }
                        finalStatePromise->set_value(state);
                    }};
            }
        }
        catch (const std::future_error&)
        {
            // NOP - received shutdown multiple times
        }
    }
    else
    {
        Logging::Warn(_logger, "lifecycle failed to shut down correctly - original shutdown reason was '{}'.",
                      std::move(reason));
    }
}

void LifecycleService::Restart(std::string /*reason*/)
{
    // Currently inoperable
    throw SilKitError("Restarting is currently deactivated.");

    //_lifecycleManagement.Restart(reason);
    //
    //if (!_hasCoordinatedSimulationStart)
    //{
    //    _lifecycleManagement->Run("LifecycleService::Restart() was called without start coordination.");
    //}
}

void LifecycleService::SetLifecycleConfiguration(LifecycleConfiguration startConfiguration)
{
    _operationMode = startConfiguration.operationMode;
    if (HasRequiredParticipantNames())
    {
        CheckForValidConfiguration();
    }
}

OperationMode LifecycleService::GetOperationMode() const
{
    return _operationMode;
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

void LifecycleService::TriggerAbortHandler(ParticipantState lastState)
{
    if (_abortHandler)
    {
        _abortHandler(lastState);
    }
}

void LifecycleService::AbortSimulation(std::string reason)
{
    _lifecycleManagement.AbortSimulation(reason);
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
               << ": This participant is in OperationMode::Coordinated but it is not part of the"
                  "set of \"required\" participants declared to the system controller. ";
            ReportError(ss.str());
            return false;
        }
    }
    return true;
}

auto LifecycleService::State() const -> ParticipantState
{
    std::unique_lock<decltype(_statusMx)> lock{_statusMx};
    return _status.state;
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

void LifecycleService::SetAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    _participant->SetAsyncSubscriptionsCompletionHandler(std::move(handler));
}

auto LifecycleService::GetTimeSyncService() -> ITimeSyncService*
{
    return _timeSyncService;
}

auto LifecycleService::CreateTimeSyncService() -> ITimeSyncService*
{
    if (!_timeSyncActive)
    {
        _timeSyncActive = true;
        return _timeSyncService;
    }
    else
    {
        throw ConfigurationError("You may not create the time synchronization service more than once.");
    }
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

void LifecycleService::SetWorkflowConfiguration(const WorkflowConfiguration& configuration)
{
    SetRequiredParticipantNames(configuration.requiredParticipantNames);
    if (_operationMode != OperationMode::Invalid)
    {
        CheckForValidConfiguration();
    }
}

void LifecycleService::ChangeState(ParticipantState newState, std::string reason)
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
    case SystemState::Running: 
        _logger->Info("Simulation running");
        break; // ignore
    case SystemState::Paused: 
        _logger->Info("Simulation paused"); 
        break; // ignore
    case SystemState::Stopping: 
        _logger->Info("Simulation stopping");
        _lifecycleManagement.SystemwideStopping(ss.str()); 
        break;
    case SystemState::Stopped:
        _lifecycleManagement.SystemwideStopping(ss.str());
        break; // To Sync or not to Sync - that is the question -- for now don't
    case SystemState::ShuttingDown:
        HandleSystemStateShuttingDown();
        _lifecycleManagement.SystemwideStopping(ss.str());
        break; // ignore
    case SystemState::Shutdown: 
        _logger->Info("Simulation has shutdown");
        HandleSystemStateShuttingDown();
        _lifecycleManagement.SystemwideStopping(ss.str());
        break; // ignore
    case SystemState::Aborting: 
        break; // ignore - we will receive an abort command separately
    case SystemState::Error: 
        break; // ignore - not relevant for lifecycle (only for simulation)
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

void LifecycleService::HandleSystemStateShuttingDown()
{
    try
    {
        _systemStateReachedShuttingDownPromise.set_value();
    }
    catch (...)
    {
        // Ignore any exception. This function will be called multiple times.
    }
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
