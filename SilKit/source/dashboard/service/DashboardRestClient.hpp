// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <memory>
#include <string>

#include "services/logging/ILoggerInternal.hpp"

#include "dashboard/DashboardBulkUpdate.hpp"
#include "dashboard/IRestClient.hpp"
#include "dashboard/client/IDashboardSystemServiceClient.hpp"
#include "dashboard/http/AsioHttpClient.hpp"
#include "dashboard/http/HttpRetryPolicy.hpp"
#include "dashboard/http/IHttpClient.hpp"
#include "dashboard/service/IDashboardDtoMapper.hpp"
#include "services/metrics/MetricsDatatypes.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardRestClient : public VSilKit::IRestClient
{
public:
    /*! Connect to the dashboard at dashboardServerUri.
     *
     *  The timeouts and retry policy are parameters so that tests can drive the assembled stack
     *  without waiting out real deadlines; production uses the defaults.
     */
    DashboardRestClient(Services::Logging::ILoggerInternal* logger, const std::string& dashboardServerUri,
                        VSilKit::AsioHttpClientTimeouts timeouts = {}, VSilKit::HttpRetryPolicy retryPolicy = {});
    ~DashboardRestClient() override;

public: // For testing
    DashboardRestClient(Services::Logging::ILoggerInternal* logger,
                        std::shared_ptr<IDashboardSystemServiceClient> serviceClient,
                        std::shared_ptr<IDashboardDtoMapper> mapper);

public: // IRestClient
    uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time) override;

    void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate) override;

    void OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate) override;

    void Abort() override;

private: //member
    Services::Logging::ILoggerInternal* _logger{nullptr};
    //! Null in tests; held only so that Abort() can reach the transport.
    std::shared_ptr<VSilKit::IHttpClient> _httpClient;
    std::shared_ptr<IDashboardDtoMapper> _dtoMapper;
    std::shared_ptr<IDashboardSystemServiceClient> _serviceClient;
};

} // namespace Dashboard
} // namespace SilKit
