// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioRegistry.hpp"

using asio::ip::tcp;

namespace ib {
namespace mw {

VAsioRegistry::VAsioRegistry(ib::cfg::Config cfg)
    : _vasioConfig{cfg.middlewareConfig.vasio}
    , _connection{std::move(cfg), "IbRegistry", 0}
{
    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement)
    {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { PeerIsShuttingDown(peer); });
}

std::future<void> VAsioRegistry::ProvideDomain(uint32_t domainId)
{
    // accept connection from participants on any interface
    auto registryPort = static_cast<uint16_t>(_vasioConfig.registry.port + domainId);
    tcp::endpoint registryEndpoint(tcp::v4(), registryPort);
    try
    {
        // FIXME: also accept connections on V6
        _connection.AcceptConnectionsOn(registryEndpoint);
    }
    catch (std::exception& e)
    {
        _connection.GetLogger()->Error("VAsioRegistry failed to create listening socket {} for domainId={}. Reason: {}",
            registryEndpoint,
            domainId,
            e.what());
        throw e;
    }
    _connection.StartIoWorker();

    return _allParticipantsDown.get_future();
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement)
{
    SendKnownParticipants(from);

    _connectedParticipants[announcement.peerInfo.participantId] = announcement.peerInfo;

    if (AllParticipantsUp())
    {
        _connection.GetLogger()->Info("All Participants up");
    }
}

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer)
{
    _connection.GetLogger()->Info("Sending known participant message to {}", peer->GetInfo().participantName);

    KnownParticipants knownParticipantsMsg;

    for (auto&& connectedParticipant : _connectedParticipants)
    {
        auto&& peerInfo = connectedParticipant.second;
        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    MessageBuffer sendBuffer;
    uint32_t msgSizePlaceholder{0u};
    sendBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbRegistryMessage
        << RegistryMessageKind::KnownParticipants
        << knownParticipantsMsg;

    peer->SendIbMsg(std::move(sendBuffer));
}

void VAsioRegistry::PeerIsShuttingDown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(peer->GetInfo().participantId);

    if (_connectedParticipants.empty())
    {
        _connection.GetLogger()->Info("All Participants down");
        _allParticipantsDown.set_value();
    }
}

bool VAsioRegistry::AllParticipantsUp() const
{
    for (auto&& participant : _connection.Config().simulationSetup.participants)
    {
        if (_connectedParticipants.count(participant.id) != 1)
        {
            return false;
        }
    }
    return true;
}

} // namespace mw
} // namespace ib

