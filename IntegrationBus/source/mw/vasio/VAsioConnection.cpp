// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"
#include "../registry/RegistryDatatypes.hpp"

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace mw {

namespace tt = util::tuple_tools;

template <class T> struct Zero { using Type = T; };

using asio::ip::tcp;

VAsioConnection::VAsioConnection(cfg::Config config, std::string participantName)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{get_by_name(_config.simulationSetup.participants, _participantName).id}
{}

VAsioConnection::~VAsioConnection()
{
    if (_ioWorker.joinable())
    {
        _ioContext.stop();
        _ioWorker.join();
    }
}

void VAsioConnection::JoinDomain(uint32_t domainId)
{
    assert(!_tcpAcceptor);

    // accept connection from other peers
    // let the operating system choose a free port
    _tcpAcceptor = std::make_unique<asio::ip::tcp::acceptor>(
        _ioContext,
        tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 0)
    );
    AcceptConnection();
    std::cout << "Listening on " << _tcpAcceptor->local_endpoint() << std::endl;

    auto registry = std::make_unique<VAsioTcpPeer>(_ioContext, this);

    registry->GetInfo().participantName = "Registry";
    registry->GetInfo().participantId = 0;
    registry->GetInfo().acceptorHost = "127.0.0.1";
    registry->GetInfo().acceptorPort = static_cast<uint16_t>(42000 + domainId);

    std::cout << "Connecting to registry!" << std::endl;
    auto remoteEndpoint = tcp::endpoint(
        asio::ip::address::from_string(registry->GetInfo().acceptorHost),
        registry->GetInfo().acceptorPort
    );
    registry->Socket().connect(remoteEndpoint);
    registry->StartAsyncRead();
    SendParticipantAnnoucement(registry.get());
    _registry = std::move(registry);
    
    StartIoWorker();    
}

auto VAsioConnection::ReceiveParticipantAnnoucement(MessageBuffer&& buffer, IVAsioPeer* peer) -> VAsioPeerInfo
{
    registry::ParticipantAnnouncement announcement;
    buffer >> announcement;

    registry::RegistryMessageHeader reference;
    if (dynamic_cast<registry::RegistryMessageHeader&>(announcement) != reference)
    {
        std::cout << "Received participant announcement message with unsupported protocol specification"
                  << std::endl;
    }

    peer->GetInfo() = announcement.localInfo;
    std::cout << "Received participant announcement of " << peer->GetInfo().participantName << std::endl;

    return announcement.localInfo;
}

void VAsioConnection::SendParticipantAnnoucement(IVAsioPeer* peer)
{
    VAsioPeerInfo localInfo{
        _participantName,
        _participantId,
        _tcpAcceptor->local_endpoint().address().to_string(),
        _tcpAcceptor->local_endpoint().port()
    };
    registry::ParticipantAnnouncement announcement;
    announcement.localInfo = localInfo;

    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{ 0u };

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << registry::RegistryMessageKind::ParticipantAnnouncement
           << announcement;

    peer->SendIbMsg(std::move(buffer));
    std::cout << "Send participant announcement to " << peer->GetInfo().participantName << std::endl;
}

void VAsioConnection::ReceiveSubscriptionSentEvent()
{
    if (_newPeerCallback)
    {
        _newPeerCallback();
    }
}

void VAsioConnection::ReceiveKnownParticpants(MessageBuffer&& buffer)
{
    registry::KnownParticipants participantsMsg;
    buffer >> participantsMsg;

    registry::RegistryMessageHeader reference;
    if (dynamic_cast<registry::RegistryMessageHeader&>(participantsMsg) != reference)
    {
        std::cout << "Received known participant message with unsupported protocol specification"
                  << std::endl;
        return;
    }

    std::cout << "Received known participant message" << std::endl;

    for (auto otherInfo : participantsMsg.participantInfos)
    {
        std::cout
            << "Connecting to " << otherInfo.participantName
            << " with Id " << otherInfo.participantId
            << " on " << otherInfo.acceptorHost << ":" << otherInfo.acceptorPort
            << std::endl;

        // make dns resolve
        tcp::resolver resolver(_ioContext);
        auto endpoints = resolver.resolve(otherInfo.acceptorHost, std::to_string(otherInfo.acceptorPort));
        tcp::endpoint remoteEndpoint = *endpoints.begin();
        
        auto peer = std::make_unique<VAsioTcpPeer>(_ioContext, this);
        peer->Socket().connect(remoteEndpoint);

        peer->GetInfo() = otherInfo;

        // we connected to other peer. tell him who we are.
        SendParticipantAnnoucement(peer.get());

        AddPeer(std::move(peer));
    }
}

void VAsioConnection::StartIoWorker()
{
    _ioWorker = std::thread{ [this]() {
        try
        {
            _ioContext.run();
            return 0;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Something went wrong: " << error.what() << std::endl;
            std::cout << "Press enter to stop the io worker thread..." << std::endl;
            std::cin.ignore();
            return -1;
        }
    } };
}

void VAsioConnection::AcceptConnection()
{
    assert(_tcpAcceptor);
    auto newConnection = std::make_shared<VAsioTcpPeer>(_tcpAcceptor->get_executor().context(), this);

    _tcpAcceptor->async_accept(newConnection->Socket(),
        [this, newConnection](const asio::error_code& error) mutable
        {
            if (!error)
            {
                std::cout << newConnection->Socket().remote_endpoint() << " connected to me!" << std::endl;
                AddPeer(std::move(newConnection));
            }
            AcceptConnection();
        }
    );
}

void VAsioConnection::AddPeer(std::shared_ptr<VAsioTcpPeer> newPeer)
{
    for (auto&& localReceiver : _rawMsgReceivers)
    {
        newPeer->Subscribe(localReceiver->GetDescriptor());
    }
    newPeer->StartAsyncRead();

    _peers.emplace_back(std::move(newPeer));   

    // notify all connected peers that new subscriptions were sent
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};
    buffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbRegistryMessage
        << registry::RegistryMessageKind::SubscriptionSent;
    for (auto&& peer : _peers)
    {
        peer->SendIbMsg(buffer);
    }
}

void VAsioConnection::RegisterNewPeerCallback(std::function<void()> callback)
{
    _newPeerCallback = callback;
}


void VAsioConnection::OnSocketData(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    VAsioMsgKind msgKind;
    buffer >> msgKind;
    switch (msgKind)
    {
    case VAsioMsgKind::Invalid:
        std::cerr << "WARNING: Received message with VAsioMsgKind::Invalid\n";
        break;
    case VAsioMsgKind::AnnounceSubscription:
        return ReceiveSubscriptionAnnouncement(std::move(buffer), peer);
    case VAsioMsgKind::IbMwMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    case VAsioMsgKind::IbSimMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    case VAsioMsgKind::IbRegistryMessage:
        return ReceiveRegistryMessage(std::move(buffer), peer);
    }
}

void VAsioConnection::ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    VAsioMsgSubscriber subscriber;
    buffer >> subscriber;

    bool wasAdded = false;

    tt::wrapped_tuple<Zero, IbMessageTypes> supportedTypes;
    tt::for_each(supportedTypes, [&](auto zero) {

        using IbMessageT = typename decltype(zero)::Type;
        wasAdded |= this->TryAddSubscriber<IbMessageT>(subscriber, peer);

    });

    if (!wasAdded)
    {
        std::cout << "WARNING: Cannot register subscriber for: [" << subscriber.linkName << "] " << subscriber.msgTypeName << "\n";
    }
}

void VAsioConnection::ReceiveRawIbMessage(MessageBuffer&& buffer)
{
    uint16_t receiverIdx;
    buffer >> receiverIdx;
    if (receiverIdx >= _rawMsgReceivers.size())
    {
        std::cerr << "WARNING: Ignoring RawIbMessage for unknown receiverIdx=" << receiverIdx << "\n";
        return;
    }
    _rawMsgReceivers[receiverIdx]->ReceiveMsg(std::move(buffer));
}

void VAsioConnection::ReceiveRegistryMessage(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    registry::RegistryMessageKind kind;
    buffer >> kind;
    switch (kind)
    {
    case registry::RegistryMessageKind::Invalid:
        std::cerr << "WARNING: Received message with RegistryMessageKind::Invalid\n";
        break;
    case registry::RegistryMessageKind::ParticipantAnnouncement:
        ReceiveParticipantAnnoucement(std::move(buffer), peer);
        break;
    case registry::RegistryMessageKind::KnownParticipants:
        ReceiveKnownParticpants(std::move(buffer));
        break;
    case registry::RegistryMessageKind::SubscriptionSent:
        ReceiveSubscriptionSentEvent();
        break;
    }
}

} // namespace mw
} // namespace ib
