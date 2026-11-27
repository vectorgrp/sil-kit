// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "services/logging/ILoggerInternal.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/vasio/VAsioConnection.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "core/internal/ProtocolVersion.hpp"
#include "services/orchestration/TimeProvider.hpp"

#include "services/metrics/MetricsReceiver.hpp"

#include "core/vasio/IRegistryEventListener.hpp"
#include "core/vasio/SessionPlacement.hpp"

namespace SilKit {
namespace Core {

struct IMsgForVAsioRegistry
    : SilKit::Core::IReceiver<SilKit::Services::Orchestration::ParticipantStatus,
                              SilKit::Services::Orchestration::WorkflowConfiguration,
                              SilKit::Core::Discovery::ServiceDiscoveryEvent>
    , SilKit::Core::ISender<>
{
};

class VAsioRegistry
    : public SilKit::Vendor::Vector::ISilKitRegistry
    , public IMsgForVAsioRegistry
    , public Core::IServiceEndpoint
    , public VSilKit::IMetricsReceiverListener
{
public: // CTor
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                  ProtocolVersion version = CurrentProtocolVersion());
    VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                  IRegistryEventListener* registryEventListener, ProtocolVersion version = CurrentProtocolVersion());

public: // methods
    auto StartListening(const std::string& listenUri) -> std::string override;

    void SetAllConnectedHandler(std::function<void()> handler) override;
    void SetAllDisconnectedHandler(std::function<void()> handler) override;

    auto GetLoggerInternal() -> Services::Logging::ILoggerInternal* ;

private:
    auto GetLogger() -> Services::Logging::ILogger* override;

private:
    // ----------------------------------------
    // private methods
    auto GetOrCreatePlacement(const std::string& simulationName) -> SessionPlacement*;
    auto FindPlacement(const std::string& simulationName) const -> SessionPlacement*;
    auto FindPlacementOfSender(const IServiceEndpoint* from) const -> SessionPlacement*;

    void OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer);
    void OnPeerShutdown(IVAsioPeer* peer);

    bool AllParticipantsAreConnected() const;

    void SetupMetrics();

private: // IReceiver<...>
    void ReceiveMsg(const IServiceEndpoint* from,
                    const SilKit::Services::Orchestration::ParticipantStatus& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from,
                    const SilKit::Services::Orchestration::WorkflowConfiguration& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const SilKit::Core::Discovery::ServiceDiscoveryEvent& msg) override;

private: // IServiceEndpoint
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override;

private: // IMetricsReceiverListener
    void OnMetricsUpdate(const std::string& simulationName, const std::string& participantName,
                         const VSilKit::MetricsUpdate& metricsUpdate) override;

private:
    // ----------------------------------------
    // private members
    std::unique_ptr<Services::Logging::ILoggerInternal> _logger;
    IRegistryEventListener* _registryEventListener{nullptr};
    std::unordered_map<std::string, std::unique_ptr<SessionPlacement>> _placements;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;
    std::shared_ptr<SilKit::Config::ParticipantConfiguration> _vasioConfig;
    Services::Orchestration::TimeProvider _timeProvider;
    std::unique_ptr<IMsgForMetricsReceiver> _metricsReceiver;
    std::unique_ptr<IMetricsProcessor> _metricsProcessor;
    std::unique_ptr<IMetricsManager> _metricsManager;
    VAsioConnection _connection;

    std::atomic<EndpointId> _localEndpointId{1};

    ServiceDescriptor _serviceDescriptor;

    friend struct VAsioRegistryTestAccess;
};

} // namespace Core
} // namespace SilKit
