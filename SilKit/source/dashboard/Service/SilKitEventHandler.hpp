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

#include "DashboardBulkUpdate.hpp"
#include "MetricsDatatypes.hpp"

#include <memory>
#include <string>

#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitToOatppMapper.hpp"
#include "IDashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {

class SilKitEventHandler
{
public:
    SilKitEventHandler(Services::Logging::ILogger* logger,
                       std::shared_ptr<IDashboardSystemServiceClient> dashboardSystemServiceClient,
                       std::shared_ptr<ISilKitToOatppMapper> silKitToOatppMapper);
    ~SilKitEventHandler();

public: //methods
    uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time);
    void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate);

    void OnMetricsUpdate(uint64_t simulationId, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate);

private: //methods
    void OnControllerCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor);
    void OnLinkCreated(uint64_t simulationId, const Core::ServiceDescriptor& serviceDescriptor);

private: //member
    Services::Logging::ILogger* _logger;
    std::shared_ptr<IDashboardSystemServiceClient> _dashboardSystemServiceClient;
    std::shared_ptr<ISilKitToOatppMapper> _silKitToOatppMapper;
};

} // namespace Dashboard
} // namespace SilKit
