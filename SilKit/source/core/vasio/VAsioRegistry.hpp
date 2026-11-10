// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <optional>
#include <unordered_map>

#include "services/logging/ILoggerInternal.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/vasio/VAsioConnection.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "core/internal/ProtocolVersion.hpp"
#include "services/orchestration/TimeProvider.hpp"
#include "services/orchestration/SystemStateTracker.hpp"

#include "services/metrics/MetricsReceiver.hpp"

#include "core/vasio/IRegistryEventListener.hpp"

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
    // private data types
    struct ConnectedParticipantInfo
    {
        IVAsioPeer* peer;
        VAsioPeerInfo peerInfo;
    };
    using ConnectedParticipants = std::unordered_map<std::string, ConnectedParticipantInfo>;
    struct Generation
    {
        ConnectedParticipants participants;
        // Used to decide when `used` below should be set.
        VSilKit::SystemStateTracker stateTracker;
        bool used{false};
    };
    using Generations = std::map<uint64_t, Generation>;
    using SimulationNames = std::unordered_map<std::string, Generations>;

private:
    // ----------------------------------------
    // private methods
    bool ShouldEnterGeneration(const std::string& participantName, const Generation& generation) const;
    auto SelectGeneration(const std::string& simulationName, const std::string& participantName) const -> uint64_t;
    //! Whether this peer, and not another one with the same name, is registered in its generation.
    //! This is most likely only defensive and not a guard that is required: the generation stays stamped on a peer
    //! after it was removed, but a peer is removed only once, on its shutdown. No path is known on which a peer that is
    //! no longer registered is shut down again while another peer with the same name is registered.
    bool IsPeerRegistered(const IVAsioPeer* peer) const;

    auto GetOrCreateGeneration(const std::string& simulationName, uint64_t generation) -> Generation&;
    auto FindGenerationOfSender(const IServiceEndpoint* from) -> Generation*;
    void UpdateGenerationUsage(const IServiceEndpoint* from, Generation& generation);

    void OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName, uint64_t generation);
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
    SimulationNames _simulationNames;
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
