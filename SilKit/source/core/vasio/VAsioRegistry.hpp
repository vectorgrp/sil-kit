// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "VAsioConnection.hpp"
#include "ParticipantConfiguration.hpp"
#include "ProtocolVersion.hpp"
#include "TimeProvider.hpp"

#include "MetricsReceiver.hpp"

namespace SilKit {
namespace Core {

struct IMsgForVAsioRegistry
    : SilKit::Core::IReceiver<SilKit::Services::Orchestration::ParticipantStatus,
                              SilKit::Services::Orchestration::WorkflowConfiguration,
                              SilKit::Core::Discovery::ServiceDiscoveryEvent>
    , SilKit::Core::ISender<>
{
};

struct IRegistryEventListener
{
    virtual ~IRegistryEventListener() = default;

    virtual void OnLoggerCreated(SilKit::Services::Logging::ILogger* logger) = 0;
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
    auto GetLogger() -> Services::Logging::ILogger* override;

private:
    // ----------------------------------------
    // private data types
    struct ConnectedParticipantInfo
    {
        IVAsioPeer* peer;
        VAsioPeerInfo peerInfo;
    };

private:
    // ----------------------------------------
    // private methods
    auto FindConnectedParticipant(const std::string& participantName,
                                  const std::string& simulationName) const -> const ConnectedParticipantInfo*;

    void OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName);
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
    std::unordered_map<std::string, std::unordered_map<std::string, ConnectedParticipantInfo>> _connectedParticipants;
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
};

} // namespace Core
} // namespace SilKit
