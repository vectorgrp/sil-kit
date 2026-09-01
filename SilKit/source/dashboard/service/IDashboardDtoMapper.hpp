// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "dashboard/DashboardBulkUpdate.hpp"
#include "dashboard/dto/BulkUpdateDto.hpp"
#include "dashboard/dto/MetricsDto.hpp"
#include "dashboard/dto/SimulationCreationRequestDto.hpp"
#include "services/metrics/MetricsDatatypes.hpp"

namespace SilKit {
namespace Dashboard {

/*! Maps SIL Kit internal types onto the dashboard's wire DTOs.
 *
 *  Only the three entry points DashboardRestClient actually needs are virtual; the per-service
 *  helpers are public non-virtual members of DashboardDtoMapper, which is what the tests exercise.
 */
class IDashboardDtoMapper
{
public:
    virtual ~IDashboardDtoMapper() = default;

    virtual auto CreateSimulationCreationRequestDto(const std::string& connectUri,
                                                    uint64_t start) -> SimulationCreationRequestDto = 0;
    virtual auto CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> BulkSimulationDto = 0;
    virtual auto CreateMetricsUpdateDto(const std::string& participantName,
                                        const VSilKit::MetricsUpdate& metricsUpdate) -> MetricsUpdateDto = 0;
};

} // namespace Dashboard
} // namespace SilKit
