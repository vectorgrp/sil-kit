#pragma once

#include <list>
#include "VAsioConnection.hpp"

namespace ib {
namespace mw {

class VAsioRegistry
{
public:
    VAsioRegistry() = delete;
    VAsioRegistry(const VAsioRegistry&) = delete;
    VAsioRegistry(VAsioRegistry&&) = delete;
    VAsioRegistry(ib::cfg::Config cfg);

    std::future<void> ProvideDomain(uint32_t domainId);

    void PeerIsShuttingDown(IVAsioPeer* peer);

    bool AllParticipantsUp() const;

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer);

private:
    // ----------------------------------------
    // private members
    std::unordered_map<ParticipantId, ib::mw::VAsioPeerInfo> _connectedParticipants;
    std::promise<void> _allParticipantsDown;

    ib::cfg::VAsio::Config _vasioConfig;
    VAsioConnection _connection;
};

} // namespace mw
} // namespace ib
