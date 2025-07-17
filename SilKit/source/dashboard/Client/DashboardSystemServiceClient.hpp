/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
