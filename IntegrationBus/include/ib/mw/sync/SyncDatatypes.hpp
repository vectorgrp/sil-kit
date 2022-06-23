// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "ib/mw/ParticipantId.hpp"

namespace ib {
namespace mw {
//! The synchronization namespace
namespace sync {

struct ParticipantCommand
{
    //! The different kinds of a ParticipantCommand
    enum class Kind : uint8_t {
        Invalid = 0, //!< An invalid command
        Initialize = 1, //!< The initialize command // TODO will be removed
        Restart = 2, //!< The restart command
        Shutdown = 3 //!< The shutdown command
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
        Shutdown = 3, //!< The shutdown command
        PrepareColdswap = 4, //!< The prepare coldswap command
        ExecuteColdswap = 5, //!< The execute coldswap command
        AbortSimulation = 6 //!< The abort simulation command
    };

    Kind kind; //!< The kind of system command that is sent.
};

// note: always increase number (never reuse old ones!)
enum class ParticipantState : uint8_t {
    Invalid = 0, //!< An invalid participant state
    ServicesCreated = 10, //!< The controllers created state
    CommunicationInitializing = 20, //!< The communication initializing state
    CommunicationInitialized = 30, //!< The communication initialized state
    ReadyToRun = 40, //!< The initialized state
    Running = 50, //!< The running state
    Paused = 60, //!< The paused state
    Stopping = 70, //!< The stopping state
    Stopped = 80, //!< The stopped state
    Error = 90, //!< The error state
    ShuttingDown = 100, //!< The shutting down state
    Shutdown = 110, //!< The shutdown state
    Reinitializing = 120,  //!< The reinitializing state

    ColdswapPrepare = 200, //!< The ColdswapPrepare state
    ColdswapReady = 210, //!< The ColdswapReady state
    ColdswapShutdown = 220, //!< The ColdswapShutdown state
    ColdswapIgnored = 230 //!< The ColdswapIgnored state
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
    Invalid = 0, //!< An invalid system state
    ServicesCreated = 10, //!< The controllers created state
    CommunicationInitializing = 20, //!< The communication initializing state
    CommunicationInitialized = 30, //!< The communication initialized state
    ReadyToRun = 40, //!< The initialized state
    Running = 50, //!< The running state
    Paused = 60, //!< The paused state
    Stopping = 70, //!< The stopping state
    Stopped = 80, //!< The stopped state
    Error = 90, //!< The error state
    ShuttingDown = 100, //!< The shutting down state
    Shutdown = 110, //!< The shutdown state
    Reinitializing = 120, //!< The reinitializing state

    ColdswapPrepare = 200, //!< The ColdswapPrepare state
    ColdswapReady = 210, //!< The ColdswapReady state
    ColdswapPending = 220, //!< The ColdswapPending state
    ColdswapDone = 230 //!< The ColdswapDone state
};

struct ExpectedParticipants
{
    std::vector<std::string> names;
};

} // namespace sync
} // namespace mw
} // namespace ib
