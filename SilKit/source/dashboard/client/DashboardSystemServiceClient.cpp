// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/client/DashboardSystemServiceClient.hpp"

#include <utility>

#include "dashboard/client/DashboardPaths.hpp"
#include "dashboard/json/DashboardJson.hpp"
#include "services/logging/LoggerMessage.hpp"

using SilKit::Services::Logging::Level;
using SilKit::Services::Logging::LoggerMessage;
using SilKit::Services::Logging::Topic;

namespace SilKit {
namespace Dashboard {

DashboardSystemServiceClient::DashboardSystemServiceClient(Services::Logging::ILoggerInternal* logger,
                                                           std::shared_ptr<VSilKit::IHttpClient> httpClient)
    : _logger(logger)
    , _httpClient(std::move(httpClient))
{
}

DashboardSystemServiceClient::~DashboardSystemServiceClient() {}

auto DashboardSystemServiceClient::CreateSimulation(const SimulationCreationRequestDto& simulation)
    -> std::optional<uint64_t>
{
    const auto result = _httpClient->Post(Paths::CreateSimulation(), ToJson(simulation));
    Log(result, "creating simulation");
    if (!result.transportError && result.statusCode == 201)
    {
        return ParseSimulationCreationResponse(result.body);
    }
    return std::nullopt;
}

void DashboardSystemServiceClient::UpdateSimulation(uint64_t simulationId, const BulkSimulationDto& bulkSimulation)
{
    const auto result = _httpClient->Post(Paths::UpdateSimulation(simulationId), ToJson(bulkSimulation));
    Log(result, "updating simulation");
}

void DashboardSystemServiceClient::UpdateSimulationMetrics(uint64_t simulationId, const MetricsUpdateDto& metrics)
{
    const auto result = _httpClient->Post(Paths::UpdateSimulationMetrics(simulationId), ToJson(metrics));
    Log(result, "updating simulation metrics");
}

void DashboardSystemServiceClient::Log(const VSilKit::HttpResult& result, const std::string& message)
{
    if (result.transportError)
    {
        _logger->MakeMessage(Level::Error, TopicOf(*this))
            .SetMessage("Dashboard: {} server unavailable", message)
            .Dispatch();
    }
    else if (result.statusCode >= 400)
    {
        _logger->MakeMessage(Level::Error, TopicOf(*this))
            .SetMessage("Dashboard: {} returned {}", message, result.statusCode)
            .Dispatch();
    }
    else
    {
        _logger->MakeMessage(Level::Debug, TopicOf(*this))
            .SetMessage("Dashboard: {} returned {}", message, result.statusCode)
            .Dispatch();
    }
}

} // namespace Dashboard
} // namespace SilKit
