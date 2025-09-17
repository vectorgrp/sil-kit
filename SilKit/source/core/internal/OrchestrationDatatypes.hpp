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

struct NextSimTask
{
    std::chrono::nanoseconds timePoint{0};
    std::chrono::nanoseconds duration{0};
};

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
