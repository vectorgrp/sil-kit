// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "silkit/services/datatypes.hpp"

#include "silkit/capi/Orchestration.h"

namespace SilKit {
namespace Services {
namespace Orchestration {

// note: never reuse old numbers!
//! \brief Available participant states
enum class ParticipantState : SilKit_ParticipantState
{
    //! An invalid participant state
    Invalid = SilKit_ParticipantState_Invalid,
    //! The controllers created state
    ServicesCreated = SilKit_ParticipantState_ServicesCreated,
    //! The communication initializing state
    CommunicationInitializing = SilKit_ParticipantState_CommunicationInitializing,
    //! The communication initialized state
    CommunicationInitialized = SilKit_ParticipantState_CommunicationInitialized,
    //! The initialized state
    ReadyToRun = SilKit_ParticipantState_ReadyToRun,
    //! The running state
    Running = SilKit_ParticipantState_Running,
    //! The paused state
    Paused = SilKit_ParticipantState_Paused,
    //! The stopping state
    Stopping = SilKit_ParticipantState_Stopping,
    //! The stopped state
    Stopped = SilKit_ParticipantState_Stopped,
    //! The error state
    Error = SilKit_ParticipantState_Error,
    //! The shutting down state
    ShuttingDown = SilKit_ParticipantState_ShuttingDown,
    //! The shutdown state
    Shutdown = SilKit_ParticipantState_Shutdown,
    //! The aborting state
    Aborting = SilKit_ParticipantState_Aborting,
};

//! \brief Details about a participant state change.
struct ParticipantStatus
{
    std::string participantName; //!< Name of the participant (UTF-8).
    ParticipantState state{ParticipantState::Invalid}; //!< The new state of the participant.
    std::string enterReason; //!< The reason for the participant to enter the new state (UTF-8).
    std::chrono::system_clock::time_point enterTime; //!< The enter time of the participant.
    std::chrono::system_clock::time_point refreshTime; //!< The refresh time.
};

// note: never reuse old numbers!
//! \brief Available system states
enum class SystemState : SilKit_SystemState
{
    //! An invalid participant state
    Invalid = SilKit_SystemState_Invalid,
    //! The controllers created state
    ServicesCreated = SilKit_SystemState_ServicesCreated,
    //! The communication initializing state
    CommunicationInitializing = SilKit_SystemState_CommunicationInitializing,
    //! The communication initialized state
    CommunicationInitialized = SilKit_SystemState_CommunicationInitialized,
    //! The initialized state
    ReadyToRun = SilKit_SystemState_ReadyToRun,
    //! The running state
    Running = SilKit_SystemState_Running,
    //! The paused state
    Paused = SilKit_SystemState_Paused,
    //! The stopping state
    Stopping = SilKit_SystemState_Stopping,
    //! The stopped state
    Stopped = SilKit_SystemState_Stopped,
    //! The error state
    Error = SilKit_SystemState_Error,
    //! The shutting down state
    ShuttingDown = SilKit_SystemState_ShuttingDown,
    //! The shutdown state
    Shutdown = SilKit_SystemState_Shutdown,
    //! The aborting state
    Aborting = SilKit_SystemState_Aborting,
};

//! \brief Details of the simulation workflow regarding lifecycle and participant coordination.
struct WorkflowConfiguration
{
    //! Participants that are waited for when coordinating the simulation start/stop.
    std::vector<std::string> requiredParticipantNames;
};

//! \brief Available operation modes of the lifecycle service
enum class OperationMode : SilKit_OperationMode
{
    //! An invalid operation mode
    Invalid = SilKit_OperationMode_Invalid,
    //! The coordinated operation mode
    Coordinated = SilKit_OperationMode_Coordinated,
    //! The autonomous operation mode
    Autonomous = SilKit_OperationMode_Autonomous,
};

//! The lifecycle start configuration.
struct LifecycleConfiguration
{
    OperationMode operationMode;
};

struct ParticipantConnectionInformation
{
    std::string participantName;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
