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

#include "silkit/services/logging/ILogger.hpp"

#include "OrchestrationDatatypes.hpp"
#include "ILifecycleStates.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//forwards
class LifecycleService;

class LifecycleManagement 
{
public: //CTors
    LifecycleManagement(Services::Logging::ILogger* logger, LifecycleService* parentService);

    void InitLifecycleManagement(std::string reason);

    void SystemwideServicesCreated(std::string reason);
    void SystemwideCommunicationInitialized(std::string reason);
    void SystemwideReadyToRun(std::string reason);
    void SystemwideStopping(std::string reason);

    void Restart(std::string reason);
    bool Shutdown(std::string reason);


    // Actions during simulation
    void Pause(std::string reason);
    void Continue(std::string reason);
    void UserStop(std::string reason);

    // Error / Abort handling
    void Error(std::string reason);
    void AbortSimulation(std::string reason);

    // uncoordinated lifecycle state initialization
    void StartUncoordinated(std::string reason);

public:
    // internal intermediate step between ServicesCreated and Comm.Initialized
    void CommunicationInitializing(std::string reason);

    void Stop(std::string reason);

    CallbackResult HandleCommunicationReady();
    bool HandleStarting();
    bool HandleStop();
    bool HandleShutdown();

    void StartRunning();

    void ContinueAfterStop();

    void ResolveAbortSimulation(std::string reason);

    // (Internal) Action after Stop
    void RestartAfterStop(std::string reason);
    void ShutdownAfterStop(std::string reason);

    // Internal state handling
    void SetState(ILifecycleState* state, std::string message);
    void SetStateError(std::string reason);

    ILifecycleState* GetCurrentState();

    ILifecycleState* GetInvalidState();
    ILifecycleState* GetOperationalState();
    ILifecycleState* GetErrorState();
    
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

    Services::Logging::ILogger* GetLogger();

    LifecycleService* GetService();

private:
    std::shared_ptr<ILifecycleState> _invalidState;
    std::shared_ptr<ILifecycleState> _operationalState;
    std::shared_ptr<ILifecycleState> _errorState;

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
    LifecycleService* _parentService;

    Services::Logging::ILogger* _logger;
    std::atomic<bool> _stopInitialized{false};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
