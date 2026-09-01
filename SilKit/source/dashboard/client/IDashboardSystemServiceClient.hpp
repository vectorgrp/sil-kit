// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>

#include "dashboard/dto/BulkUpdateDto.hpp"
#include "dashboard/dto/MetricsDto.hpp"
#include "dashboard/dto/SimulationCreationRequestDto.hpp"

namespace SilKit {
namespace Dashboard {

class IDashboardSystemServiceClient
{
public:
    virtual ~IDashboardSystemServiceClient() = default;

    /*! Register a new simulation and return the id the dashboard assigned to it.
     *
     *  std::nullopt means no id was obtained - a non-201 response, a transport failure, or a body
     *  that could not be parsed.
     */
    virtual auto CreateSimulation(const SimulationCreationRequestDto& simulation) -> std::optional<uint64_t> = 0;

    virtual void UpdateSimulation(uint64_t simulationId, const BulkSimulationDto& bulkSimulation) = 0;
    virtual void UpdateSimulationMetrics(uint64_t simulationId, const MetricsUpdateDto& metrics) = 0;

    //! Probe whether the dashboard service supports the bulk-update endpoint.
    virtual auto CheckBulkUpdateSupported() -> bool = 0;
};

} // namespace Dashboard
} // namespace SilKit
