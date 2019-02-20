// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace mw {
namespace sync {


struct QuantumRequest
{
    std::chrono::nanoseconds now;
    std::chrono::nanoseconds duration;
};

enum class QuantumRequestStatus
{
    Invalid, //!< Conversion Error
    Granted, //!< Request was granted.
    Rejected //!< Request was rejected, e.g., due to a stop.
};

struct QuantumGrant
{
    EndpointAddress grantee;
    std::chrono::nanoseconds now;
    std::chrono::nanoseconds duration;

    QuantumRequestStatus status{QuantumRequestStatus::Invalid};
};


struct Tick
{
    std::chrono::nanoseconds now;
};

struct TickDone
{
};


struct ParticipantCommand
{
    enum class Kind {
        Invalid,
        Initialize,
        ReInitialize
    };

    ParticipantId participant;
    Kind kind;
};

struct SystemCommand
{
    enum class Kind {
        Invalid,
        Run,
        Stop,
        Shutdown
    };

    Kind kind;
};

enum class ParticipantState {
    Invalid,
    Idle,
    Initializing,
    Initialized,
    Running,
    Paused,
    Stopping,
    Stopped,
    Error,
    ShuttingDown,
    Shutdown
};

struct ParticipantStatus
{
    std::string participantName;
    ParticipantState state{ParticipantState::Invalid};
    std::string enterReason;
    std::chrono::system_clock::time_point enterTime;
};

enum class SystemState {
    Invalid,
    Idle,
    Initializing,
    Initialized,
    Running,
    Paused,
    Stopping,
    Stopped,
    Error,
    ShuttingDown,
    Shutdown
};


} // namespace sync
} // namespace mw
} // namespace ib
