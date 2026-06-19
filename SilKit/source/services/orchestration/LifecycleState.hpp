// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

enum class CallbackResult
{
    Error,
    Completed,
    Deferred
};

enum class LifecycleState
{
    Invalid,
    ServicesCreated,
    CommunicationInitializing,
    CommunicationInitialized,
    ReadyToRun,
    Running,
    Paused,
    Stopping,
    Stopped,
    ShuttingDown,
    Shutdown,
    Aborting,
    Error,
};

inline auto ToParticipantState(LifecycleState state) -> ParticipantState
{
    switch (state)
    {
    case LifecycleState::Invalid:
        return ParticipantState::Invalid;
    case LifecycleState::ServicesCreated:
        return ParticipantState::ServicesCreated;
    case LifecycleState::CommunicationInitializing:
        return ParticipantState::CommunicationInitializing;
    case LifecycleState::CommunicationInitialized:
        return ParticipantState::CommunicationInitialized;
    case LifecycleState::ReadyToRun:
        return ParticipantState::ReadyToRun;
    case LifecycleState::Running:
        return ParticipantState::Running;
    case LifecycleState::Paused:
        return ParticipantState::Paused;
    case LifecycleState::Stopping:
        return ParticipantState::Stopping;
    case LifecycleState::Stopped:
        return ParticipantState::Stopped;
    case LifecycleState::ShuttingDown:
        return ParticipantState::ShuttingDown;
    case LifecycleState::Shutdown:
        return ParticipantState::Shutdown;
    case LifecycleState::Aborting:
        return ParticipantState::Aborting;
    case LifecycleState::Error:
        return ParticipantState::Error;
    }

    return ParticipantState::Invalid;
}

inline auto to_string(LifecycleState state) -> std::string
{
    switch (state)
    {
    case LifecycleState::Invalid:
        return "Invalid";
    case LifecycleState::ServicesCreated:
        return "ServicesCreated";
    case LifecycleState::CommunicationInitializing:
        return "CommunicationInitializing";
    case LifecycleState::CommunicationInitialized:
        return "CommunicationInitialized";
    case LifecycleState::ReadyToRun:
        return "ReadyToRun";
    case LifecycleState::Running:
        return "Running";
    case LifecycleState::Paused:
        return "Paused";
    case LifecycleState::Stopping:
        return "Stopping";
    case LifecycleState::Stopped:
        return "Stopped";
    case LifecycleState::ShuttingDown:
        return "ShuttingDown";
    case LifecycleState::Shutdown:
        return "Shutdown";
    case LifecycleState::Aborting:
        return "Aborting";
    case LifecycleState::Error:
        return "Error";
    }

    return "Invalid";
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
