// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DashboardSystemServiceClient.hpp"

#include "LoggerMessage.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

using namespace std::chrono_literals;
using SilKit::Services::Logging::Level;
using SilKit::Services::Logging::Topic;
using SilKit::Services::Logging::LoggerMessage;

namespace SilKit {
namespace Dashboard {

DashboardSystemServiceClient::DashboardSystemServiceClient(
    Services::Logging::ILoggerInternal* logger, std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
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
        _logger->MakeMessage(Level::Error, TopicOf(*this))
            .SetMessage("Dashboard: {} server unavailable", message)
            .Dispatch();
    }
    else if (response->getStatusCode() >= 400)
    {
        _logger->MakeMessage(Level::Error, TopicOf(*this))
            .SetMessage("Dashboard: {} returned {}", message, response->getStatusCode())
            .Dispatch();
    }
    else
    {
        _logger->MakeMessage(Level::Debug, TopicOf(*this))
            .SetMessage("Dashboard: {} returned {}", message, response->getStatusCode())
            .Dispatch();
    }
}

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)
