#pragma once

#include <list>

#include "VAsioConnection.hpp"
#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace mw {

class VAsioRegistry
{
public:
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(ib::cfg::Config cfg);

    void ProvideDomain(uint32_t domainId);

    void SetAllConnectedHandler(std::function<void()> handler);
    void SetAllDisconnectedHandler(std::function<void()> handler);

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement);
    bool IsExpectedParticipant(const ib::mw::VAsioPeerInfo& peerInfo);
    void SendKnownParticipants(IVAsioPeer* peer);
    void OnPeerShutdown(IVAsioPeer* peer);

    bool AllParticipantsAreConnected() const;

private:
    // ----------------------------------------
    // private members
    std::unique_ptr<logging::ILogger> _logger;
    std::unordered_map<ParticipantId, ib::mw::VAsioPeerInfo> _connectedParticipants;
    std::function<void()> _onAllParticipantsConnected;
    std::function<void()> _onAllParticipantsDisconnected;

    ib::cfg::VAsio::Config _vasioConfig;
    VAsioConnection _connection;
};

} // namespace mw
} // namespace ib
