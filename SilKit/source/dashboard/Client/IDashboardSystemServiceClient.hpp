// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "SimulationCreationRequestDto.hpp"
#include "SimulationCreationResponseDto.hpp"
#include "BulkUpdateDto.hpp"
#include "MetricsDto.hpp"

namespace SilKit {
namespace Dashboard {

class IDashboardSystemServiceClient
{
public:
    virtual ~IDashboardSystemServiceClient() = default;

    virtual oatpp::Object<SimulationCreationResponseDto> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) = 0;

    virtual void UpdateSimulation(oatpp::UInt64 simulationId, oatpp::Object<BulkSimulationDto> bulkSimulation) = 0;
    virtual void UpdateSimulationMetrics(oatpp::UInt64 simulationId, oatpp::Object<MetricsUpdateDto> metrics) = 0;
};

} // namespace Dashboard
} // namespace SilKit
