// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/SilKit.hpp"
#include "DashboardRestClient.hpp"
#include "DashboardComponents.hpp"
#include "LoggerMessage.hpp"
#include "Uri.hpp"
#include "SilKitToOatppMapper.hpp"
#include "Client/DashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {

DashboardRestClient::DashboardRestClient(Services::Logging::ILogger* logger, const std::string& dashboardServerUri)
    : _logger(logger)
{
    oatpp::base::Environment::init();
    auto uri = SilKit::Core::Uri::Parse(dashboardServerUri);
    SilKit::Dashboard::DashboardComponents dashboardComponents{uri.Host(), uri.Port()};
    auto objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>);
    _retryPolicy = std::make_shared<SilKit::Dashboard::DashboardRetryPolicy>(3);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, connectionProvider);
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider, _retryPolicy);
    _silKitToOatppMapper = std::make_shared<SilKit::Dashboard::SilKitToOatppMapper>();
    _apiClient = SilKit::Dashboard::DashboardSystemApiClient::createShared(requestExecutor, objectMapper);
    auto serviceClient =
        std::make_shared<SilKit::Dashboard::DashboardSystemServiceClient>(_logger, _apiClient, objectMapper);
    _serviceClient = serviceClient;
}

DashboardRestClient::~DashboardRestClient()
{
    if (_retryPolicy != nullptr)
    {
        _retryPolicy->AbortAllRetries();
        _retryPolicy.reset();
    }
    _silKitToOatppMapper.reset();
    _apiClient.reset();
    _serviceClient.reset();
    oatpp::base::Environment::destroy();
}

bool DashboardRestClient::IsBulkUpdateSupported()
{
    auto bulkSimulationDto = SilKit::Dashboard::BulkSimulationDto::createShared();
    const auto response = _apiClient->updateSimulation(oatpp::UInt64{std::uint64_t{0}}, bulkSimulationDto);
    if (response)
    {
        const auto statusCode = response->getStatusCode();
        return  200 <= statusCode && statusCode < 300;
    }
    return false;
}

uint64_t DashboardRestClient::OnSimulationStart(const std::string& connectUri, uint64_t time)
{
    Services::Logging::Info(_logger, "Dashboard: creating simulation {} {}", connectUri, time);
    auto simulation = _serviceClient->CreateSimulation(
        _silKitToOatppMapper->CreateSimulationCreationRequestDto(connectUri, time));
    if (simulation)
    {
        Services::Logging::Info(_logger, "Dashboard: created simulation with id {}", *simulation->id.get());
        return simulation->id;
    }
    _logger->Warn("Dashboard: creating simulation failed");
    return 0;
}

void DashboardRestClient::OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate)
{
    _serviceClient->UpdateSimulation(simulationId, _silKitToOatppMapper->CreateBulkSimulationDto(bulkUpdate));
}

void DashboardRestClient::OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                                         const VSilKit::MetricsUpdate& metricsUpdate)
{
    _serviceClient->UpdateSimulationMetrics(simulationId,
                                            _silKitToOatppMapper->CreateMetricsUpdateDto(origin, metricsUpdate));
}

} // namespace Dashboard
} // namespace SilKit
