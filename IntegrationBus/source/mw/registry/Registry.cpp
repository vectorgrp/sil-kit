// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Registry.hpp"

using namespace ib::mw;
using namespace ib::mw::registry;

using asio::ip::tcp;

Registry::Registry(ib::cfg::Config cfg)
{
    _config = cfg;
}

std::future<void> Registry::ProvideDomain(uint32_t domainId)
{
    assert(!_tcpAcceptor);

    // accept connection from all participants
    // at the moment registry listens on 0.0.0.0:(42000+domainId)
    auto registryPort = static_cast<uint16_t>(42000 + domainId);
    _tcpAcceptor = std::make_unique<asio::ip::tcp::acceptor>(_ioContext, tcp::endpoint(tcp::v4(), registryPort));
    AcceptConnection();
    std::cout << "Listening on " << _tcpAcceptor->local_endpoint() << std::endl;

    StartIoWorker();

    return _allParticipantsDown.get_future();
}

auto Registry::ReceiveParticipantAnnoucement(MessageBuffer&& buffer, IVAsioPeer* peer) -> VAsioPeerInfo
{
    auto info = VAsioConnection::ReceiveParticipantAnnoucement(std::move(buffer), peer);

    registry::KnownParticipants knownParticipantsMsg;
    for (auto && connectedParticipant : _connectedParticipants)
    {
        knownParticipantsMsg.participantInfos.push_back(connectedParticipant.second);
    }

    _connectedParticipants[info.participantId] = info;

    MessageBuffer sendBuffer;
    uint32_t msgSizePlaceholder{ 0u };
    sendBuffer << msgSizePlaceholder
               << VAsioMsgKind::IbRegistryMessage
               << registry::RegistryMessageKind::KnownParticipants
               << knownParticipantsMsg;
    peer->SendIbMsg(std::move(sendBuffer));
    std::cout << "Send known participant message" << std::endl;

    bool allParticipantsUp = true;
    for (auto&& participant : _config.simulationSetup.participants)
    {
        allParticipantsUp &= _connectedParticipants.find(participant.id) != _connectedParticipants.end();
    }
    if (allParticipantsUp)
    {
        std::cout << "All Participants up" << std::endl;
    }

    return info;
}

void Registry::PeerIsShuttingDown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(peer->GetInfo().participantId);

    if (_connectedParticipants.empty())
    {
        std::cout << "All Participants down" << std::endl;
    }
}