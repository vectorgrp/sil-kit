// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include "dashboard/client/IDashboardSystemServiceClient.hpp"
#include "dashboard/http/IHttpClient.hpp"
#include "services/logging/ILoggerInternal.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    DashboardSystemServiceClient(Services::Logging::ILoggerInternal* logger,
                                 std::shared_ptr<VSilKit::IHttpClient> httpClient);
    ~DashboardSystemServiceClient() override;

    auto CreateSimulation(const SimulationCreationRequestDto& simulation) -> std::optional<uint64_t> override;
    void UpdateSimulation(uint64_t simulationId, const BulkSimulationDto& bulkSimulation) override;
    void UpdateSimulationMetrics(uint64_t simulationId, const MetricsUpdateDto& metrics) override;
    auto CheckBulkUpdateSupported() -> bool override;

private:
    void Log(const VSilKit::HttpResult& result, const std::string& message);

    Services::Logging::ILoggerInternal* _logger;
    std::shared_ptr<VSilKit::IHttpClient> _httpClient;
};

} // namespace Dashboard
} // namespace SilKit
