// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DashboardRestClient.hpp"

namespace SilKit {
namespace Dashboard {

DashboardRestClient::DashboardRestClient(Services::Logging::ILogger* logger, const std::string& dashboardServerUri)
    : _logger(logger)
{
    // Constructor implementation
}

DashboardRestClient::~DashboardRestClient()
{
    // Destructor implementation
}

bool DashboardRestClient::IsBulkUpdateSupported()
{
    // Method implementation
    return true;
}

uint64_t DashboardRestClient::OnSimulationStart(const std::string& connectUri, uint64_t time)
{
    // Method implementation
    return 0;
}

void DashboardRestClient::OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate)
{
    // Method implementation
}

void DashboardRestClient::OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                                          const VSilKit::MetricsUpdate& metricsUpdate)
{
    // Method implementation
}

} // namespace Dashboard
} // namespace SilKit
