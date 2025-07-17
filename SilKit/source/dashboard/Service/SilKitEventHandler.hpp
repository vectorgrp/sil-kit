// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "DashboardBulkUpdate.hpp"
#include "MetricsDatatypes.hpp"

#include <memory>
#include <string>

#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitToOatppMapper.hpp"
#include "IDashboardSystemServiceClient.hpp"
#include "DashboardSystemApiClient.hpp"
#include "DashboardRetryPolicy.hpp"

namespace SilKit {
namespace Dashboard {

class SilKitEventHandler
{
public:
    SilKitEventHandler(Services::Logging::ILogger* logger, const std::string& dashboardServerUri);
    ~SilKitEventHandler();
public: //methods
    bool IsBulkUpdateSupported();
    uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time);
    void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate);

    void OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate);

private: //member
    Services::Logging::ILogger* _logger;
    std::shared_ptr<SilKit::Dashboard::DashboardRetryPolicy> _retryPolicy;
    std::shared_ptr<ISilKitToOatppMapper> _silKitToOatppMapper;
    std::shared_ptr<DashboardSystemApiClient> _apiClient;
    std::shared_ptr<IDashboardSystemServiceClient> _serviceClient;
};

} // namespace Dashboard
} // namespace SilKit
