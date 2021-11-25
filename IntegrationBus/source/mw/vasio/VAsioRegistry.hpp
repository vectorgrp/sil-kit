// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <list>

#include "VAsioConnection.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/extensions/IIbRegistry.hpp"

namespace ib {
namespace mw {

class VAsioRegistry
    : public ib::extensions::IIbRegistry
{
public:
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(ib::cfg::Config cfg);

    void ProvideDomain(uint32_t domainId) override;

    void SetAllConnectedHandler(std::function<void()> handler) override;
    void SetAllDisconnectedHandler(std::function<void()> handler) override;

    auto GetLogger() -> logging::ILogger* override;

private:
    // ----------------------------------------
    // private data types
    struct ConnectedParticipantInfo {
        IVAsioPeer* peer;
        ib::mw::VAsioPeerUri peerUri;
    };

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement);
    bool IsExpectedParticipant(const ib::mw::VAsioPeerUri& peerInfo);
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

    ib::cfg::VAsio::Config _vasioConfig;
    VAsioConnection _connection;
};

} // namespace mw
} // namespace ib
