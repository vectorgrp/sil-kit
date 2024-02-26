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

#include <unordered_map>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "VAsioConnection.hpp"
#include "ParticipantConfiguration.hpp"
#include "ProtocolVersion.hpp"
#include "TimeProvider.hpp"

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
};

class VAsioRegistry
    : public SilKit::Vendor::Vector::ISilKitRegistry
    , public IMsgForVAsioRegistry
    , public Core::IServiceEndpoint
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
    auto FindConnectedParticipant(const std::string& participantName, const std::string& simulationName) const
        -> const ConnectedParticipantInfo*;

    void OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName);
    void OnPeerShutdown(IVAsioPeer* peer);

    bool AllParticipantsAreConnected() const;

private: // IReceiver<...>
    void ReceiveMsg(const IServiceEndpoint* from,
                    const SilKit::Services::Orchestration::ParticipantStatus& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from,
                    const SilKit::Services::Orchestration::WorkflowConfiguration& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const SilKit::Core::Discovery::ServiceDiscoveryEvent& msg) override;

private: // IServiceEndpoint
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override;

private:
    // ----------------------------------------
    // private members
    std::unique_ptr<Services::Logging::ILogger> _logger;
    IRegistryEventListener* _registryEventListener{nullptr};
    std::unordered_map<std::string, std::unordered_map<std::string, ConnectedParticipantInfo>> _connectedParticipants;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;
    std::shared_ptr<SilKit::Config::ParticipantConfiguration> _vasioConfig;
    Services::Orchestration::TimeProvider _timeProvider;
    VAsioConnection _connection;

    ServiceDescriptor _serviceDescriptor;
};

} // namespace Core
} // namespace SilKit
