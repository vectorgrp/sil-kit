/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "OatppHeaders.hpp"

#include "SimulationCreationRequestDto.hpp"
#include "SystemStatusDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "ServiceDto.hpp"
#include "DataPublisherDto.hpp"
#include "DataSubscriberDto.hpp"
#include "RpcClientDto.hpp"
#include "RpcServerDto.hpp"
#include "SimulationEndDto.hpp"
#include "BulkUpdateDto.hpp"

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
