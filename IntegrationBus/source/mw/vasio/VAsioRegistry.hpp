// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <list>

#include "VAsioConnection.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/vendor/IIbRegistry.hpp"
#include "ParticipantConfiguration.hpp"
#include "ProtocolVersion.hpp"

namespace ib {
namespace mw {

class VAsioRegistry
    : public ib::vendor::IIbRegistry
{
public: // CTor
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> cfg,
        ProtocolVersion version =CurrentProtocolVersion());

public: // methods
    void ProvideDomain(uint32_t domainId) override;

    void ProvideDomain(std::string listenUri) override;

    void SetAllConnectedHandler(std::function<void()> handler) override;
    void SetAllDisconnectedHandler(std::function<void()> handler) override;
    auto GetLogger() -> logging::ILogger* override;

private:
    // ----------------------------------------
    // private data types
    struct ConnectedParticipantInfo {
        IVAsioPeer* peer;
        ib::mw::VAsioPeerInfo peerInfo;
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
    std::unique_ptr<logging::ILogger> _logger;
    std::vector<ConnectedParticipantInfo> _connectedParticipants;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;
    std::shared_ptr<ib::cfg::ParticipantConfiguration> _vasioConfig;
    VAsioConnection _connection;
};

} // namespace mw
} // namespace ib
