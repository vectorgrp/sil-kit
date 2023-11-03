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

#pragma once

#include <memory>
#include <atomic>
#include <mutex>

#include "silkit/services/logging/ILogger.hpp"

#include "OrchestrationDatatypes.hpp"
#include "ILifecycleStates.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//forwards
class LifecycleService;

class LifecycleManagement 
{
public: //CTors
    LifecycleManagement(
        Core::IParticipantInternal* participant, 
        Services::Logging::ILogger* logger,
        LifecycleService* parentService);

    // Triggered by Public API calls
    void Pause(std::string reason);            // ILifecycleService::Pause
    void Continue(std::string reason);         // ILifecycleService::Continue
    void Stop(std::string reason);             // ILifecycleService::Stop
    void AbortSimulation(std::string reason);  // ISystemController::AbortSimulation
    void Error(std::string reason);            // ILifecycleService::ReportError

    // Currently not part of ILifecycleService
    void Restart(std::string reason);          // LifecycleService::Restart

    // Common internal actions
    void Initialize(std::string reason);
    void ServicesCreated(std::string reason);
    void ReadyToRun(std::string reason);
    void CommunicationInitialized(std::string reason);
    void CompleteCommunicationReadyHandler(std::string reason);
    void CommunicationInitializing(std::string reason);
    void Shutdown(std::string reason);

    // Autonomous lifecycle state initialization
    void StartAutonomous(std::string reason);

    // Send NextSimTask
    void StartTime();

    // Callback handling
    CallbackResult HandleCommunicationReady();
    bool HandleStarting();
    bool HandleStop();
    bool HandleShutdown();
    bool HandleAbort();

    // Wait for pending subscriptions before advancing from CommunicationInitializing to CommunicationInitialized
    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler);

    // Abort handling
    void ResolveAbortSimulation(std::string reason);

    // Actions after Stop
    void RestartAfterStop(std::string reason);
    void ShutdownAfterAbort(std::string reason);

    // Ignore peer disconnects after stop
    void NotifyShutdownInConnection();

    // State setter
    void SetState(ILifecycleState* newState, std::string message);

    // Set state and trigger an action on the new state.
    void SetStateAndForwardIntent(
        ILifecycleState* nextState, 
        void (ILifecycleState::*intent)(std::string),
        std::string reason);

    // State getter
    ILifecycleState* GetCurrentState();
    ILifecycleState* GetInvalidState();
    ILifecycleState* GetOperationalState();
    ILifecycleState* GetErrorState();
    ILifecycleState* GetAbortingState();
    ILifecycleState* GetServicesCreatedState();
    ILifecycleState* GetCommunicationInitializingState();
    ILifecycleState* GetCommunicationInitializedState();
    ILifecycleState* GetReadyToRunState();
    ILifecycleState* GetRunningState();
    ILifecycleState* GetPausedState();
    ILifecycleState* GetStoppingState();
    ILifecycleState* GetStoppedState();
    ILifecycleState* GetShuttingDownState();
    ILifecycleState* GetShutdownState();

    // Property getters
    OperationMode GetOperationMode() const;

    // Interface getters
    Logging::ILogger* GetLogger();
    LifecycleService* GetService();
    Core::IParticipantInternal* GetParticipant();

private:
    void UpdateLifecycleState(ILifecycleState* newState);
    void UpdateParticipantState(std::string reason);

    Core::IParticipantInternal* _participant{nullptr};

    std::shared_ptr<ILifecycleState> _invalidState;
    std::shared_ptr<ILifecycleState> _operationalState;
    std::shared_ptr<ILifecycleState> _errorState;
    std::shared_ptr<ILifecycleState> _abortingState;
    std::shared_ptr<ILifecycleState> _servicesCreatedState;
    std::shared_ptr<ILifecycleState> _communicationInitializingState;
    std::shared_ptr<ILifecycleState> _communicationInitializedState;
    std::shared_ptr<ILifecycleState> _readyToRunState;
    std::shared_ptr<ILifecycleState> _runningState;
    std::shared_ptr<ILifecycleState> _pausedState;
    std::shared_ptr<ILifecycleState> _stoppingState;
    std::shared_ptr<ILifecycleState> _stoppedState;
    std::shared_ptr<ILifecycleState> _shuttingDownState;
    std::shared_ptr<ILifecycleState> _shutDownState;

    ILifecycleState* _currentState;
    ILifecycleState* _lastBeforeAbortingState{nullptr};
    LifecycleService* _lifecycleService;

    Services::Logging::ILogger* _logger;
    std::recursive_mutex _mutex;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
