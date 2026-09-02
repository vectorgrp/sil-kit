// SPDX-FileCopyrightText: 2022-2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/service/DashboardRestClient.hpp"

#include <utility>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "dashboard/client/DashboardSystemServiceClient.hpp"
#include "dashboard/http/AsioHttpClient.hpp"
#include "dashboard/http/RetryingHttpClient.hpp"
#include "dashboard/service/DashboardDtoMapper.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "util/Uri.hpp"

using SilKit::Services::Logging::Level;
using SilKit::Services::Logging::LoggerMessage;
using SilKit::Services::Logging::Topic;

namespace SilKit {
namespace Dashboard {

DashboardRestClient::DashboardRestClient(Services::Logging::ILoggerInternal* logger,
                                         const std::string& dashboardServerUri,
                                         VSilKit::AsioHttpClientTimeouts timeouts,
                                         VSilKit::HttpRetryPolicy retryPolicy)
    : _logger(logger)
{
    _dtoMapper = std::make_shared<DashboardDtoMapper>(logger);

    const auto uri = SilKit::Core::Uri::Parse(dashboardServerUri);
    auto transport = std::make_shared<VSilKit::AsioHttpClient>(logger, uri.Host(), uri.Port(), timeouts);
    _httpClient = std::make_shared<VSilKit::RetryingHttpClient>(std::move(transport), retryPolicy);
    _serviceClient = std::make_shared<DashboardSystemServiceClient>(_logger, _httpClient);
}

DashboardRestClient::DashboardRestClient(Services::Logging::ILoggerInternal* logger,
                                         std::shared_ptr<IDashboardSystemServiceClient> serviceClient,
                                         std::shared_ptr<IDashboardDtoMapper> mapper)
    : _logger(logger)
    , _dtoMapper(std::move(mapper))
    , _serviceClient(std::move(serviceClient))
{
}

DashboardRestClient::~DashboardRestClient()
{
    Abort();
    _dtoMapper.reset();
    _serviceClient.reset();
    _httpClient.reset();
}

void DashboardRestClient::Abort()
{
    if (_httpClient != nullptr)
    {
        _httpClient->Abort();
    }
}

uint64_t DashboardRestClient::OnSimulationStart(const std::string& connectUri, uint64_t time)
{
    _logger->MakeMessage(Level::Info, TopicOf(*this))
        .SetMessage("Dashboard: creating simulation {} {}", connectUri, time)
        .Dispatch();
    const auto simulationId =
        _serviceClient->CreateSimulation(_dtoMapper->CreateSimulationCreationRequestDto(connectUri, time));
    if (simulationId.has_value())
    {
        _logger->MakeMessage(Level::Info, TopicOf(*this))
            .SetMessage("Dashboard: created simulation with id {}", *simulationId)
            .Dispatch();
        return *simulationId;
    }
    _logger->MakeMessage(Level::Warn, TopicOf(*this))
        .SetMessage("Dashboard: creating simulation failed")
        .Dispatch();
    return 0;
}

void DashboardRestClient::OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate)
{
    _serviceClient->UpdateSimulation(simulationId, _dtoMapper->CreateBulkSimulationDto(bulkUpdate));
}

void DashboardRestClient::OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                                          const VSilKit::MetricsUpdate& metricsUpdate)
{
    _serviceClient->UpdateSimulationMetrics(simulationId, _dtoMapper->CreateMetricsUpdateDto(origin, metricsUpdate));
}

} // namespace Dashboard
} // namespace SilKit
