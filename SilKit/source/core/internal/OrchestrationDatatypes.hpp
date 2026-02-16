// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

enum class SynchronizationKind: uint8_t
{
    None = 0,
    RequestSynchronization = 1,
    AcknowledgeSynchronization = 2,
};

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
    SynchronizationKind synchronizationKind{SynchronizationKind::None};
    uint64_t serialNumber{0};
};

inline auto operator==(const NextSimTask& lhs, const NextSimTask& rhs)
{
    return lhs.duration == rhs.duration && lhs.timePoint == rhs.timePoint;
}

//! System-wide command for the simulation flow.
struct SystemCommand
{
    //!< The different kinds of a SystemCommand
    enum class Kind : uint8_t
    {
        Invalid = 0,        //!< An invalid command
        AbortSimulation = 1 //!< The abort simulation command
    };

    Kind kind; //!< The kind of system command that is sent.
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
