// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/mw/logging/ILogger.hpp"

#include "LifecycleService.hpp"
#include "SyncDatatypes.hpp"
#include "ILifecycleStates.hpp"

namespace ib {
namespace mw {
namespace sync {

class ILifecycleManagement
{
public:
    virtual ~ILifecycleManagement() = default;

public:
    virtual void InitLifecycleManagement(std::string reason) = 0;
    virtual void SkipSetupPhase(std::string reason) = 0;
    virtual void NewSystemState(SystemState systemState) = 0;
    virtual void Run(std::string reason) = 0;
    virtual void Pause(std::string reason) = 0;
    virtual void Continue(std::string reason) = 0;
    virtual void Stop(std::string reason) = 0;
    virtual bool Shutdown(std::string reason) = 0; // shutdown successful?
    virtual void Restart(std::string reason) = 0;
    virtual void Error(std::string reason) = 0;
    virtual bool AbortSimulation(std::string reason) = 0; // Abort->Shutdown successful?
};

class LifecycleManagement 
    : public ILifecycleManagement
{
public:
    LifecycleManagement(logging::ILogger* logger, LifecycleService* parentService);

    void InitLifecycleManagement(std::string reason) override;
    void SkipSetupPhase(std::string reason) override;
    void NewSystemState(SystemState systemState) override;
    void Run(std::string reason) override;
    void Pause(std::string reason) override;
    void Continue(std::string reason) override;
    void Stop(std::string reason) override;
    bool Shutdown(std::string reason) override;
    void Restart(std::string reason) override;
    void Error(std::string reason) override;
    bool AbortSimulation(std::string reason) override;

public:
    void HandleCommunicationReady(std::string reason);
    void HandleStarting(std::string reason);
    void HandleStop(std::string reason);

    void HandleShutdown(std::string reason);

    // Internal state handling
    void SetState(ILifecycleState* state, std::string message);
    void SetStateError(std::string reason);

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

    logging::ILogger* GetLogger();

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

    logging::ILogger* _logger;
};


} // namespace sync
} // namespace mw
} // namespace ib
