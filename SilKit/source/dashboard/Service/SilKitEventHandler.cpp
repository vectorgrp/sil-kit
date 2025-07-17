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

#include "SilKitEventHandler.hpp"

#include "LoggerMessage.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/SilKit.hpp"
#include "Uri.hpp"

namespace SilKit {
namespace Dashboard {

SilKitEventHandler::SilKitEventHandler(Services::Logging::ILogger* logger,
                                       std::shared_ptr<IDashboardSystemServiceClient> dashboardSystemServiceClient,
                                       std::shared_ptr<ISilKitToOatppMapper> silKitToOatppMapper)
    : _logger(logger)
    , _dashboardSystemServiceClient(dashboardSystemServiceClient)
    , _silKitToOatppMapper(silKitToOatppMapper)
{
}

SilKitEventHandler::~SilKitEventHandler() {}

uint64_t SilKitEventHandler::OnSimulationStart(const std::string& connectUri, uint64_t time)
{
    Services::Logging::Info(_logger, "Dashboard: creating simulation {} {}", connectUri, time);
    auto simulation = _dashboardSystemServiceClient->CreateSimulation(
        _silKitToOatppMapper->CreateSimulationCreationRequestDto(connectUri, time));
    if (simulation)
    {
        Services::Logging::Info(_logger, "Dashboard: created simulation with id {}", *simulation->id.get());
        return simulation->id;
    }
    _logger->Warn("Dashboard: creating simulation failed");
    return 0;
}

void SilKitEventHandler::OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate)
{
    _dashboardSystemServiceClient->UpdateSimulation(simulationId,
                                                    _silKitToOatppMapper->CreateBulkSimulationDto(bulkUpdate));
}

void SilKitEventHandler::OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                                         const VSilKit::MetricsUpdate& metricsUpdate)
{
    _dashboardSystemServiceClient->UpdateSimulationMetrics(
        simulationId, _silKitToOatppMapper->CreateMetricsUpdateDto(origin, metricsUpdate));
}


void SilKitEventHandler::OnControllerCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
    Services::Logging::Debug(_logger, "Dashboard: adding service for simulation {} {}", simulationId,
                             serviceDescriptor);
}

void SilKitEventHandler::OnLinkCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor)
{
}

} // namespace Dashboard
} // namespace SilKit
