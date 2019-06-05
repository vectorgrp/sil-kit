// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {
//! The synchronization namespace
namespace sync {

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
};

struct QuantumRequest
{
    std::chrono::nanoseconds now; //!< Quantum request time
    std::chrono::nanoseconds duration; //!< Requested quantum duration
};

enum class QuantumRequestStatus : uint8_t
{
    Invalid, //!< Conversion Error
    Granted, //!< Request was granted.
    Rejected //!< Request was rejected, e.g., due to a stop.
};

struct QuantumGrant
{
    EndpointAddress grantee; //!< Endpoint of the grantee
    std::chrono::nanoseconds now; //!< Quantum grant time
    std::chrono::nanoseconds duration; //!< Granted quantum duration

    QuantumRequestStatus status{QuantumRequestStatus::Invalid}; //!< Status of the quantum request
};


struct Tick
{
    std::chrono::nanoseconds now; //!< Current tick time
    std::chrono::nanoseconds duration; //!< Tick duration
};

struct TickDone
{
    Tick finishedTick; //!< Represents a finished tick
};

struct ParticipantCommand
{
    //! The different kinds of a ParticipantCommand
    enum class Kind : uint8_t {
        Invalid, //!< An invalid command
        Initialize, //!< The initialize command
        ReInitialize //!< The re-inizialize command
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
    Invalid, //!< An invalid participant state
    Idle, //!< The idle state
    Initializing, //!< The initializing state
    Initialized, //!< The initialized state
    Running, //!< The running state
    Paused, //!< The paused state
    Stopping, //!< The stopping state
    Stopped, //!< The stopped state
    ColdswapPrepare, //!< The ColdswapPrepare state
    ColdswapReady, //!< The ColdswapReady state
    ColdswapShutdown, //!< The ColdswapShutdown state
    ColdswapIgnored, //!< The ColdswapIgnored state
    Error, //!< The error state
    ShuttingDown, //!< The ShuttingDown state
    Shutdown //!< The shutdown state
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
    ShuttingDown, //!< The ShuttingDown state
    Shutdown //!< The shutdown state
};


} // namespace sync
} // namespace mw
} // namespace ib
