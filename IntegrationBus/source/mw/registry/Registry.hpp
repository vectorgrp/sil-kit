#pragma once

#include <list>
#include "../vasio/VAsioConnection.hpp"

namespace ib {
namespace mw {
namespace registry {

class Registry
    : public VAsioConnection
{
public:
    Registry() = delete;
    Registry(const Registry&) = delete;
    Registry(Registry&&) = delete;
    Registry(ib::cfg::Config cfg);

    std::future<void> ProvideDomain(uint32_t domainId);

    void PeerIsShuttingDown(IVAsioPeer* peer) override;

private:
    auto ReceiveParticipantAnnoucement(MessageBuffer&& buffer, IVAsioPeer* peer) -> VAsioPeerInfo override;

private:
    std::unordered_map<ParticipantId, ib::mw::VAsioPeerInfo> _connectedParticipants;
    std::promise<void> _allParticipantsDown;
};

} // namespace registry
} // namespace mw
} // namespace ib