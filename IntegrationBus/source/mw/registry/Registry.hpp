#pragma once

#include <list>
#include "VAsioConnection.hpp"

namespace ib {
namespace mw {
namespace registry {

class Registry
{
public:
    Registry() = delete;
    Registry(const Registry&) = delete;
    Registry(Registry&&) = delete;
    Registry(ib::cfg::Config cfg);

    std::future<void> ProvideDomain(uint32_t domainId);

    void PeerIsShuttingDown(IVAsioPeer* peer);

    bool AllParticipantsUp() const;

private:
    // ----------------------------------------
    // private methods
    void OnParticipantAnnouncement(IVAsioPeer* from, const registry::ParticipantAnnouncement& announcement);
    void SendKnownParticipants(IVAsioPeer* peer);

private:
    // ----------------------------------------
    // private members
    std::unordered_map<ParticipantId, ib::mw::VAsioPeerInfo> _connectedParticipants;
    std::promise<void> _allParticipantsDown;

    VAsioConnection _connection;
};

} // namespace registry
} // namespace mw
} // namespace ib