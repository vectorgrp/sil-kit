// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

namespace SilKit {
namespace Dashboard {
namespace Paths {

// No leading '/': the transport prepends it, as the previous oatpp ApiClient did.

inline auto CreateSimulation() -> std::string
{
    return "system-service/v1.0/simulations";
}

inline auto UpdateSimulation(uint64_t simulationId) -> std::string
{
    return "system-service/v1.1/simulations/" + std::to_string(simulationId);
}

inline auto UpdateSimulationMetrics(uint64_t simulationId) -> std::string
{
    return "system-service/v1.1/simulations/" + std::to_string(simulationId) + "/metrics";
}

} // namespace Paths
} // namespace Dashboard
} // namespace SilKit
