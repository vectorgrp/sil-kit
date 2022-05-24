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
        Invalid, //!< An invalid command
        Initialize, //!< The initialize command
        Reinitialize, //!< The reinitialize command
        Shutdown //!< The shutdown command
    };

    ParticipantId participant; //!< The specific participant that receives this command.
    Kind kind; //!< The kind of participant command that is sent.
};

struct SystemCommand
{
    //! The different kinds of a SystemCommand
    enum class Kind : uint8_t {
        Invalid, //!< An invalid command
        Run, //!< The run command
        Stop, //!< The stop command
        Shutdown, //!< The shutdown command
        PrepareColdswap, //!< The prepare coldswap command
        ExecuteColdswap //!< The execute coldswap command
    };

    Kind kind; //!< The kind of system command that is sent.
};

enum class ParticipantState : uint8_t {
    Invalid = 0, //!< An invalid participant state
    Idle = 1, //!< The idle state
    Initializing = 2, //!< The initializing state
    Initialized = 3, //!< The initialized state
    Running = 4, //!< The running state
    Paused = 5, //!< The paused state
    Stopping = 6, //!< The stopping state
    Stopped = 7, //!< The stopped state
    ColdswapPrepare = 8, //!< The ColdswapPrepare state
    ColdswapReady = 9, //!< The ColdswapReady state
    ColdswapShutdown = 10, //!< The ColdswapShutdown state
    ColdswapIgnored = 11, //!< The ColdswapIgnored state
    Error = 12, //!< The error state
    ShuttingDown = 13, //!< The shutting down state
    Shutdown = 14, //!< The shutdown state
    Reinitializing = 15  //!< The reinitializing state
};

struct ParticipantStatus
{
    std::string participantName; //!< Name of the participant.
    ParticipantState state{ParticipantState::Invalid}; //!< The new state of the participant.
    std::string enterReason; //!< The reason for the participant to enter the new state.
    std::chrono::system_clock::time_point enterTime; //!< The enter time of the participant.
    std::chrono::system_clock::time_point refreshTime; //!< The refresh time.
};

enum class SystemState : uint8_t {
    Invalid, //!< An invalid system state
    Idle, //!< The idle state
    Initializing, //!< The initializing state
    Initialized, //!< The initialized state
    Running, //!< The running state
    Paused, //!< The paused state
    Stopping, //!< The stopping state
    Stopped, //!< The stopped state
    ColdswapPrepare, //!< The ColdswapPrepare state
    ColdswapReady, //!< The ColdswapReady state
    ColdswapPending, //!< The ColdswapPending state
    ColdswapDone, //!< The ColdswapDone state
    Error, //!< The error state
    ShuttingDown, //!< The shutting down state
    Shutdown //!< The shutdown state
};

struct ExpectedParticipants
{
    std::vector<std::string> names;
};

} // namespace sync
} // namespace mw
} // namespace ib
