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
        Initialize = 1, //!< The initialize command
        Reinitialize = 2, //!< The reinitialize command
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
        ExecuteColdswap = 5 //!< The execute coldswap command
    };

    Kind kind; //!< The kind of system command that is sent.
};

// note: always increase number (never reuse old ones!)
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

// note: always increase number (never reuse old ones!)
enum class SystemState : uint8_t {
    Invalid = 0, //!< An invalid system state
    Idle = 1, //!< The idle state
    Initializing = 2, //!< The initializing state
    Initialized = 3, //!< The initialized state
    Running = 4, //!< The running state
    Paused = 5, //!< The paused state
    Stopping = 6, //!< The stopping state
    Stopped = 7, //!< The stopped state
    ColdswapPrepare = 8, //!< The ColdswapPrepare state
    ColdswapReady = 9, //!< The ColdswapReady state
    ColdswapPending = 10, //!< The ColdswapPending state
    ColdswapDone = 11, //!< The ColdswapDone state
    Error = 12, //!< The error state
    ShuttingDown = 13, //!< The shutting down state
    Shutdown = 14, //!< The shutdown state
    Reinitializing = 15 //!< The reinitializing state
};

struct ExpectedParticipants
{
    std::vector<std::string> names;
};

} // namespace sync
} // namespace mw
} // namespace ib
