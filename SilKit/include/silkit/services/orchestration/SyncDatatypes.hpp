// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "silkit/ParticipantId.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

struct ParticipantCommand
{
    //! The different kinds of a ParticipantCommand
    enum class Kind : uint8_t {
        Invalid = 0, //!< An invalid command
        Restart = 1, //!< The restart command
        Shutdown = 2 //!< The shutdown command
    };

    ParticipantId participant; //!< The specific participant that receives this command.
    Kind kind; //!< The kind of participant command that is sent.
};

struct SystemCommand
{
    //! The different kinds of a SystemCommand
    enum class Kind : uint8_t {
        Invalid = 0, //!< An invalid command
        Run = 1, //!< The run command
        Stop = 2, //!< The stop command
        AbortSimulation = 6 //!< The abort simulation command
    };

    Kind kind; //!< The kind of system command that is sent.
};

// note: always increase number (never reuse old ones!)
enum class ParticipantState : uint8_t {
    Invalid                     =   0, //!< An invalid participant state
    ServicesCreated             =  10, //!< The controllers created state
    CommunicationInitializing   =  20, //!< The communication initializing state
    CommunicationInitialized    =  30, //!< The communication initialized state
    ReadyToRun                  =  40, //!< The initialized state
    Running                     =  50, //!< The running state
    Paused                      =  60, //!< The paused state
    Stopping                    =  70, //!< The stopping state
    Stopped                     =  80, //!< The stopped state
    Error                       =  90, //!< The error state
    ShuttingDown                = 100, //!< The shutting down state
    Shutdown                    = 110, //!< The shutdown state
    Reinitializing              = 120, //!< The reinitializing state
};

struct ParticipantStatus
{
    std::string participantName; //!< Name of the participant.
    ParticipantState state{ParticipantState::Invalid}; //!< The new state of the participant.
    std::string enterReason; //!< The reason for the participant to enter the new state.
    std::chrono::system_clock::time_point enterTime; //!< The enter time of the participant.
    std::chrono::system_clock::time_point refreshTime; //!< The refresh time.
};

// note: always increase number (never reuse old ones!)
enum class SystemState : uint8_t {
    Invalid                     =   0, //!< An invalid participant state
    ServicesCreated             =  10, //!< The controllers created state
    CommunicationInitializing   =  20, //!< The communication initializing state
    CommunicationInitialized    =  30, //!< The communication initialized state
    ReadyToRun                  =  40, //!< The initialized state
    Running                     =  50, //!< The running state
    Paused                      =  60, //!< The paused state
    Stopping                    =  70, //!< The stopping state
    Stopped                     =  80, //!< The stopped state
    Error                       =  90, //!< The error state
    ShuttingDown                = 100, //!< The shutting down state
    Shutdown                    = 110, //!< The shutdown state
    Reinitializing              = 120, //!< The reinitializing state
};

//! \brief Details of the simulation workflow regarding lifecycle and participant coordination.
struct WorkflowConfiguration
{
    std::vector<std::string> requiredParticipantNames; //!< Participants that are waited for when coordinating the simulation start/stop.
};

//!< The lifecycle start configuration.
struct LifecycleConfiguration
{
    bool coordinatedStart;
    bool coordinatedStop;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
