// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Registry.hpp"

using namespace ib::mw;
using namespace ib::mw::registry;

using asio::ip::tcp;

Registry::Registry(ib::cfg::Config cfg)
    : _vasioConfig{cfg.middlewareConfig.vasio}
    , _connection{std::move(cfg), "VibRegistry", 0}
{
    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const registry::ParticipantAnnouncement& announcement)
    {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { PeerIsShuttingDown(peer); });
}

std::future<void> Registry::ProvideDomain(uint32_t domainId)
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
        std::cout
            << "ERROR: Registry failed to create listening socket: " << registryEndpoint
            << " for domainId=" << domainId
            << ". Reason:" << e.what() << std::endl;
        throw e;
    }
    _connection.StartIoWorker();

    return _allParticipantsDown.get_future();
}

void Registry::OnParticipantAnnouncement(IVAsioPeer* from, const registry::ParticipantAnnouncement& announcement)
{
    SendKnownParticipants(from);

    _connectedParticipants[announcement.peerInfo.participantId] = announcement.peerInfo;

    if (AllParticipantsUp())
    {
        std::cout << "INFO: All Participants up" << std::endl;
    }
}

void Registry::SendKnownParticipants(IVAsioPeer* peer)
{
    std::cout << "INFO: Sending known participant message to " << peer->GetInfo().participantName << "\n";

    registry::KnownParticipants knownParticipantsMsg;

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
        << registry::RegistryMessageKind::KnownParticipants
        << knownParticipantsMsg;

    peer->SendIbMsg(std::move(sendBuffer));
}

void Registry::PeerIsShuttingDown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(peer->GetInfo().participantId);

    if (_connectedParticipants.empty())
    {
        std::cout << "INFO: All Participants down" << std::endl;
        _allParticipantsDown.set_value();
    }
}

bool Registry::AllParticipantsUp() const
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