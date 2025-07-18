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

#include <string>

namespace SilKit {
namespace Dashboard {

class ISilKitEventHandler
{
public:
    virtual ~ISilKitEventHandler() = default;
    virtual uint64_t OnSimulationStart(const std::string& connectUri, uint64_t time) = 0;
    virtual void OnSimulationEnd(uint64_t simulationId, uint64_t time) = 0;
    virtual void OnParticipantConnected(
        uint64_t simulationId,
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation) = 0;
    virtual void OnParticipantStatusChanged(uint64_t simulationId,
                                            const Services::Orchestration::ParticipantStatus& participantStatus) = 0;
    virtual void OnSystemStateChanged(uint64_t simulationId, Services::Orchestration::SystemState systemState) = 0;
    virtual void OnServiceDiscoveryEvent(uint64_t simulationId,
                                         Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                         const Core::ServiceDescriptor& serviceDescriptor) = 0;

    virtual void OnBulkUpdate(uint64_t simulationId, const DashboardBulkUpdate& bulkUpdate) = 0;

    virtual void OnMetricsUpdate(uint64_t simulationId, const std::string &origin,
                                 const VSilKit::MetricsUpdate& metricsUpdate) = 0;
};

} // namespace Dashboard
} // namespace SilKit
