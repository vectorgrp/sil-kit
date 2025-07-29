// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <string>

namespace SilKit {
namespace Services {
namespace Orchestration {

enum class CallbackResult
{
    Error,
    Completed,
    Deferred
};

class ILifecycleState
{
public:
    virtual ~ILifecycleState() = default;

    // Advance from Invalid to ServicesCreated; Fail otherwise.
    virtual void Initialize(std::string reason) = 0;

    // Autonomous: Directly...
    // Coordinated: Wait for SystemState::ServicesCreated, then...
    // ... advance to CommunicationInitializingState after receiving replies from all other participants.
    virtual void ServicesCreated(std::string reason) = 0;

    // Advance to CommunicationInitialized after all pending service subscriptions are completed.
    virtual void CommunicationInitializing(std::string reason) = 0;

    // Autonomous: Directly...
    // Coordinated: Wait for SystemState::CommunicationInitialized, then...
    // ... trigger CommunicationReadyHandler; If completed, advance to ReadyToRun.
    virtual void CommunicationInitialized(std::string reason) = 0;

    // Deferred CommunicationReadyHandler completion, advance to ReadyToRun
    virtual void CompleteCommunicationReadyHandler(std::string reason) = 0;

    // Autonomous: Directly...
    // Coordinated: Wait for SystemState::ReadyToRun, then...
    // ... trigger StartingHandler for participants without active timeSync; If completed, advance to ReadyToRun.
    virtual void ReadyToRun(std::string reason) = 0;

    virtual void RunSimulation(std::string reason) = 0;

    virtual void PauseSimulation(std::string reason) = 0;

    virtual void ContinueSimulation(std::string reason) = 0;

    // Autonomous: Directly...
    // Coordinated: Wait for SystemState::Stopping, then...
    // ... trigger StopHandler; If completed, advance to Stopped. Then,
    // advance to ShuttingDown after receiving replies from all other participants.
    virtual void StopSimulation(std::string reason) = 0;

    // Trigger the ShutdownHandler; If completed, advance to ShutdownState.
    virtual void ShutdownParticipant(std::string reason) = 0;

    virtual void AbortSimulation(std::string reason) = 0;
    virtual void ResolveAbortSimulation(std::string reason) = 0;

    virtual void Error(std::string reason) = 0;

    virtual auto toString() -> std::string = 0;
    virtual auto GetParticipantState() -> ParticipantState = 0;

    // Currently not implemented
    virtual void RestartParticipant(std::string reason) = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
