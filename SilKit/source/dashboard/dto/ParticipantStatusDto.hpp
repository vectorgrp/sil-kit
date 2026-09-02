// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Dashboard {

//! Wire representation of SilKit::Services::Orchestration::ParticipantState. Serialized as its name.
enum class ParticipantState : int32_t
{
    Unknown = -1,
    Invalid = 0,
    ServicesCreated = 10,
    CommunicationInitializing = 20,
    CommunicationInitialized = 30,
    ReadyToRun = 40,
    Running = 50,
    Paused = 60,
    Stopping = 70,
    Stopped = 80,
    Error = 90,
    ShuttingDown = 100,
    Shutdown = 110,
    Aborting = 120,
};

inline auto ToStringView(ParticipantState state) -> std::string_view
{
    switch (state)
    {
    case ParticipantState::Unknown:
        return "unknown";
    case ParticipantState::Invalid:
        return "invalid";
    case ParticipantState::ServicesCreated:
        return "servicescreated";
    case ParticipantState::CommunicationInitializing:
        return "communicationinitializing";
    case ParticipantState::CommunicationInitialized:
        return "communicationinitialized";
    case ParticipantState::ReadyToRun:
        return "readytorun";
    case ParticipantState::Running:
        return "running";
    case ParticipantState::Paused:
        return "paused";
    case ParticipantState::Stopping:
        return "stopping";
    case ParticipantState::Stopped:
        return "stopped";
    case ParticipantState::Error:
        return "error";
    case ParticipantState::ShuttingDown:
        return "shuttingdown";
    case ParticipantState::Shutdown:
        return "shutdown";
    case ParticipantState::Aborting:
        return "aborting";
    }
    throw SilKitError{"Dashboard: invalid ParticipantState"};
}

struct ParticipantStatusDto
{
    ParticipantState state{ParticipantState::Invalid};
    //! Reason for entering the state.
    std::string enterReason;
    //! Time when the state was entered.
    uint64_t enterTime{};
};

} // namespace Dashboard
} // namespace SilKit
