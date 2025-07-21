// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include "SimulationCreationRequestDto.hpp"
#include "BulkUpdateDto.hpp"
#include "MetricsDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

namespace SilKit {
namespace Dashboard {

class DashboardSystemApiClient : public oatpp::web::client::ApiClient
{
    API_CLIENT_INIT(DashboardSystemApiClient)

    // notify a simulation has been started
    // get a simulationId in return that can be used to send additional data
    API_CALL("POST", "system-service/v1.0/simulations", createSimulation,
             BODY_DTO(Object<SimulationCreationRequestDto>, simulation))

    // bulk update of a simulation
    API_CALL("POST", "system-service/v1.1/simulations/{simulationId}", updateSimulation, PATH(UInt64, simulationId),
             BODY_DTO(Object<BulkSimulationDto>, simulation))

    // bulk update of simulation metrics
    API_CALL("POST", "system-service/v1.1/simulations/{simulationId}/metrics", updateSimulationMetrics,
             PATH(UInt64, simulationId), BODY_DTO(Object<MetricsUpdateDto>, simulation))
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)
