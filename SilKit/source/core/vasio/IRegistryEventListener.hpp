// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/service/ServiceDatatypes.hpp"
#include "services/logging/ILoggerInternal.hpp"
#include "services/metrics/MetricsDatatypes.hpp"
#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Core {

/*! Everything the registry reports about the simulations it is hosting.
 *
 *  Split out of VAsioRegistry.hpp so that a listener implementation does not have to include the
 *  whole registry (and with it VAsioConnection and ParticipantConfiguration) just to declare itself.
 *
 *  The registry holds its listener as a raw, non-owning pointer and calls every method below from
 *  its asio I/O thread, except OnLoggerCreated and OnRegistryUri, which happen earlier on the
 *  thread that constructs and starts the registry. A listener must therefore outlive the registry.
 */
struct IRegistryEventListener
{
    virtual ~IRegistryEventListener() = default;

    virtual void OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal* logger) = 0;
    virtual void OnRegistryUri(const std::string& registryUri) = 0;
    virtual void OnParticipantConnected(const std::string& simulationName, const std::string& participantName) = 0;
    virtual void OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName) = 0;
    virtual void OnRequiredParticipantsUpdate(const std::string& simulationName, const std::string& participantName,
                                              SilKit::Util::Span<const std::string> requiredParticipantNames) = 0;
    virtual void OnParticipantStatusUpdate(
        const std::string& simulationName, const std::string& participantName,
        const SilKit::Services::Orchestration::ParticipantStatus& participantStatus) = 0;
    virtual void OnServiceDiscoveryEvent(
        const std::string& simulationName, const std::string& participantName,
        const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent) = 0;
    virtual void OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                                 const VSilKit::MetricsUpdate& metricsUpdate) = 0;
};

} // namespace Core
} // namespace SilKit
