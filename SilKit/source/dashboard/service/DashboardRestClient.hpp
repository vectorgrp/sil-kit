// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <memory>
#include <string>

#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitToOatppMapper.hpp"
#include "IDashboardSystemServiceClient.hpp"
#include "DashboardSystemApiClient.hpp"
#include "DashboardRetryPolicy.hpp"
#include "IRestClient.hpp"
#include "DashboardBulkUpdate.hpp"
#include "MetricsDatatypes.hpp"

namespace SilKit {
namespace Dashboard {

// Utility to initialize the Oatpp library separately, e.g. in test cases
struct LibraryInitializer
{
    LibraryInitializer();
    ~LibraryInitializer();
};

class DashboardRestClient : public VSilKit::IRestClient
{
public:
    DashboardRestClient(Services::Logging::ILogger* logger, const std::string& dashboardServerUri);
    ~DashboardRestClient() override;

public: // For testing
    DashboardRestClient(std::shared_ptr<LibraryInitializer> libraryInit, Services::Logging::ILogger* logger,
                        std::shared_ptr<IDashboardSystemServiceClient> serviceClient,
                        std::shared_ptr<ISilKitToOatppMapper> mapper);

public: // IRestClient
    bool IsBulkUpdateSupported() override;
    uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time) override;

    void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate) override;

    void OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate) override;

private: //member
    std::shared_ptr<LibraryInitializer> _libraryInit;
    Services::Logging::ILogger* _logger;
    std::shared_ptr<SilKit::Dashboard::DashboardRetryPolicy> _retryPolicy;
    std::shared_ptr<ISilKitToOatppMapper> _silKitToOatppMapper;
    std::shared_ptr<DashboardSystemApiClient> _apiClient;
    std::shared_ptr<IDashboardSystemServiceClient> _serviceClient;
};

} // namespace Dashboard
} // namespace SilKit
