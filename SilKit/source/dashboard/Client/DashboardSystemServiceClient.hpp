// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IDashboardSystemServiceClient.hpp"

#include <memory>

#include "silkit/services/logging/ILogger.hpp"

#include "DashboardSystemApiClient.hpp"

namespace SilKit {
namespace Dashboard {

class DashboardSystemServiceClient : public IDashboardSystemServiceClient
{
public:
    DashboardSystemServiceClient(Services::Logging::ILogger* logger,
                                 std::shared_ptr<DashboardSystemApiClient> dashboardSystemApiClient,
                                 std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper);
    ~DashboardSystemServiceClient();

public:
    oatpp::Object<SimulationCreationResponseDto> CreateSimulation(
        oatpp::Object<SimulationCreationRequestDto> simulation) override;
    void UpdateSimulation(oatpp::UInt64 simulationId, oatpp::Object<BulkSimulationDto> bulkSimulation) override;
    void UpdateSimulationMetrics(oatpp::UInt64 simulationId, oatpp::Object<MetricsUpdateDto> metrics) override;

private:
    void Log(std::shared_ptr<oatpp::web::client::RequestExecutor::Response> response, const std::string& message);

private:
    Services::Logging::ILogger* _logger;
    std::shared_ptr<DashboardSystemApiClient> _dashboardSystemApiClient;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
};

} // namespace Dashboard
} // namespace SilKit
