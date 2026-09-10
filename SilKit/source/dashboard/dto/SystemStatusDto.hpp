// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string_view>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Dashboard {

//! Wire representation of SilKit::Services::Orchestration::SystemState. Serialized as its name.
enum class SystemState : int32_t
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

inline auto ToStringView(SystemState state) -> std::string_view
{
    switch (state)
    {
    case SystemState::Unknown:
        return "unknown";
    case SystemState::Invalid:
        return "invalid";
    case SystemState::ServicesCreated:
        return "servicescreated";
    case SystemState::CommunicationInitializing:
        return "communicationinitializing";
    case SystemState::CommunicationInitialized:
        return "communicationinitialized";
    case SystemState::ReadyToRun:
        return "readytorun";
    case SystemState::Running:
        return "running";
    case SystemState::Paused:
        return "paused";
    case SystemState::Stopping:
        return "stopping";
    case SystemState::Stopped:
        return "stopped";
    case SystemState::Error:
        return "error";
    case SystemState::ShuttingDown:
        return "shuttingdown";
    case SystemState::Shutdown:
        return "shutdown";
    case SystemState::Aborting:
        return "aborting";
    }
    throw SilKitError{"Dashboard: invalid SystemState"};
}

struct SystemStatusDto
{
    SystemState state{SystemState::Invalid};
};

} // namespace Dashboard
} // namespace SilKit
