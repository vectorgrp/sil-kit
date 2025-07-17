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

#include "DashboardSystemServiceClient.hpp"

#include "LoggerMessage.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

using namespace std::chrono_literals;

namespace SilKit {
namespace Dashboard {

DashboardSystemServiceClient::DashboardSystemServiceClient(
    Services::Logging::ILogger* logger, std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper)
    : _logger(logger)
    , _dashboardSystemApiClient(dashboardSystemApiClient)
    , _objectMapper(objectMapper)
{
}

DashboardSystemServiceClient::~DashboardSystemServiceClient() {}

void DashboardSystemServiceClient::UpdateSimulation(oatpp::UInt64 simulationId,
                                                    oatpp::Object<BulkSimulationDto> bulkSimulation)
{
    auto response = _dashboardSystemApiClient->updateSimulation(simulationId, bulkSimulation);
    Log(response, "updating simulation");
}

oatpp::Object<SimulationCreationResponseDto> DashboardSystemServiceClient::CreateSimulation(
    oatpp::Object<SimulationCreationRequestDto> simulation)
{
    auto response = _dashboardSystemApiClient->createSimulation(simulation);
    Log(response, "creating simulation");
    if (response && response->getStatusCode() == 201)
    {
        return response->readBodyToDto<oatpp::Object<SimulationCreationResponseDto>>(_objectMapper);
    }
    return nullptr;
}

void DashboardSystemServiceClient::UpdateSimulationMetrics(oatpp::UInt64 simulationId,
                                                           oatpp::Object<MetricsUpdateDto> metrics)
{
    auto response = _dashboardSystemApiClient->updateSimulationMetrics(simulationId, metrics);
    Log(response, "updating simulation metrics");
}


void DashboardSystemServiceClient::Log(std::shared_ptr<oatpp::web::client::RequestExecutor::Response> response,
                                       const std::string& message)
{
    if (!response)
    {
        Services::Logging::Error(_logger, "Dashboard: {} server unavailable", message);
    }
    else if (response->getStatusCode() >= 400)
    {
        Services::Logging::Error(_logger, "Dashboard: {} returned {}", message, response->getStatusCode());
    }
    else
    {
        Services::Logging::Debug(_logger, "Dashboard: {} returned {}", message, response->getStatusCode());
    }
}

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)
