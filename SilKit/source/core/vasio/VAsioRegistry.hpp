// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <list>

#include "VAsioConnection.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "ParticipantConfiguration.hpp"
#include "ProtocolVersion.hpp"

namespace SilKit {
namespace Core {

class VAsioRegistry
    : public SilKit::Vendor::Vector::ISilKitRegistry
{
public: // CTor
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                  ProtocolVersion version = CurrentProtocolVersion());

public: // methods
    void ProvideDomain(const std::string& listenUri) override;

    void SetAllConnectedHandler(std::function<void()> handler) override;
    void SetAllDisconnectedHandler(std::function<void()> handler) override;
    auto GetLogger() -> Services::Logging::ILogger* override;

private:
    // ----------------------------------------
    // private data types
    struct ConnectedParticipantInfo {
        IVAsioPeer* peer;
        SilKit::Core::VAsioPeerInfo peerInfo;
    };

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement);
    auto FindConnectedPeer(const std::string& name) const->std::vector<ConnectedParticipantInfo>::const_iterator;
    void SendKnownParticipants(IVAsioPeer* peer);
    void OnPeerShutdown(IVAsioPeer* peer);

    bool AllParticipantsAreConnected() const;

private:
    // ----------------------------------------
    // private members
    std::unique_ptr<Services::Logging::ILogger> _logger;
    std::vector<ConnectedParticipantInfo> _connectedParticipants;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;
    std::shared_ptr<SilKit::Config::ParticipantConfiguration> _vasioConfig;
    VAsioConnection _connection;
};

} // namespace Core
} // namespace SilKit
