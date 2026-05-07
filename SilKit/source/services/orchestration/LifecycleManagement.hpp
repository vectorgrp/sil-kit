// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>

#include "silkit/services/logging/ILogger.hpp"

#include "OrchestrationDatatypes.hpp"
#include "LifecycleState.hpp"
#include "LifecycleStateMachine.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//forwards
class LifecycleService;

class LifecycleManagement
{
public: //CTors
    LifecycleManagement(Core::IParticipantInternal* participant, Services::Logging::ILogger* logger,
                        LifecycleService* parentService);

    // Triggered by Public API calls
    void Pause(std::string reason);           // ILifecycleService::Pause
    void Continue(std::string reason);        // ILifecycleService::Continue
    void Stop(std::string reason);            // ILifecycleService::Stop
    void AbortSimulation(std::string reason); // ISystemController::AbortSimulation
    void Error(std::string reason);           // ILifecycleService::ReportError

    // Currently not part of ILifecycleService
    void Restart(std::string reason); // LifecycleService::Restart

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

    auto CurrentState() const -> LifecycleState;

    // Machine effects
    OperationMode GetOperationMode() const;
    auto IsTimeSyncActive() const -> bool;
    void ChangeParticipantState(ParticipantState newState, std::string reason);
    auto HandleCommunicationReady() -> CallbackResult;
    auto HandleStarting() -> bool;
    auto HandleStop() -> bool;
    auto HandleShutdown() -> bool;
    auto HandleAbort(ParticipantState lastState) -> bool;
    void StartTime();
    void StopTime();
    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler);
    void CallAfterAllParticipantsReplied(std::function<void()> handler);
    void NotifyShutdownInConnection();
    void SetFinalStatePromise();
    auto GetLogger() -> Logging::ILogger*;

    Core::IParticipantInternal* _participant{nullptr};
    LifecycleService* _lifecycleService;

    Services::Logging::ILogger* _logger;
    LifecycleStateMachine<LifecycleManagement> _stateMachine;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
