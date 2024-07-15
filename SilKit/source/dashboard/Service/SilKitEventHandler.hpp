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

#include "ISilKitEventHandler.hpp"

#include <memory>

#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitToOatppMapper.hpp"
#include "IDashboardSystemServiceClient.hpp"

namespace SilKit {
namespace Dashboard {

class SilKitEventHandler : public ISilKitEventHandler
{
public:
    SilKitEventHandler(Services::Logging::ILogger* logger,
                       std::shared_ptr<IDashboardSystemServiceClient> dashboardSystemServiceClient,
                       std::shared_ptr<ISilKitToOatppMapper> silKitToOatppMapper);
    ~SilKitEventHandler();

public: //methods
    uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time) override;
    void OnSimulationEnd(uint64_t simulationId, uint64_t time) override;
    void OnParticipantConnected(
        uint64_t simulationId,
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation) override;
    void OnParticipantStatusChanged(uint64_t simulationId,
                                    const Services::Orchestration::ParticipantStatus& participantStatus) override;
    void OnSystemStateChanged(uint64_t simulationId, Services::Orchestration::SystemState systemState) override;
    void OnServiceDiscoveryEvent(uint64_t simulationId, Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                 const Core::ServiceDescriptor& serviceDescriptor) override;
    void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate) override;

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
