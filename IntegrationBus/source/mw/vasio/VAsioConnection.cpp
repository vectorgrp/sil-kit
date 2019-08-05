// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"

#include "ib/cfg/string_utils.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace ib {
namespace mw {

namespace tt = util::tuple_tools;

template <class T> struct Zero { using Type = T; };

using asio::ip::tcp;

VAsioConnection::VAsioConnection(cfg::Config config, std::string participantName, ParticipantId participantId)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
{
}

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
    // We let the operating system choose a free port
    AcceptConnectionsOn(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 0));

    auto& vasioConfig = _config.middlewareConfig.vasio;

    VAsioPeerInfo registryInfo;
    registryInfo.participantName = "VibRegistry";
    registryInfo.participantId = 0;
    registryInfo.acceptorHost = vasioConfig.registry.hostname;
    registryInfo.acceptorPort = static_cast<uint16_t>(vasioConfig.registry.port + domainId);

    std::cout << "INFO: Connecting to registry\n";

    auto registry = std::make_unique<VAsioTcpPeer>(_ioContext, this);
    registry->Connect(std::move(registryInfo));
    // FIXME: throw on connection failure!
    registry->StartAsyncRead();

    SendParticipantAnnoucement(registry.get());
    _registry = std::move(registry);
    
    StartIoWorker();    
}

void VAsioConnection::ReceiveParticipantAnnouncement(IVAsioPeer* from, MessageBuffer&& buffer)
{
    ParticipantAnnouncement announcement;
    buffer >> announcement;

    RegistryMsgHeader reference;
    if (announcement.messageHeader != reference)
    {
        std::cerr << "WARNING: Received participant announcement message with unsupported protocol specification\n";
        return;
    }

    std::cout << "INFO: Received participant announcement from " << announcement.peerInfo.participantName << std::endl;

    from->SetInfo(announcement.peerInfo);
    for (auto&& receiver : _participantAnnouncementReceivers)
    {
        receiver(from, announcement);
    }
}

void VAsioConnection::SendParticipantAnnoucement(IVAsioPeer* peer)
{
    VAsioPeerInfo localInfo{
        _participantName,
        _participantId,
        _tcpAcceptor->local_endpoint().address().to_string(),
        _tcpAcceptor->local_endpoint().port()
    };

    ParticipantAnnouncement announcement;
    announcement.peerInfo = localInfo;

    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncement
           << announcement;

    std::cout << "Sending participant announcement to " << peer->GetInfo().participantName << std::endl;
    peer->SendIbMsg(std::move(buffer));
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
    KnownParticipants participantsMsg;
    buffer >> participantsMsg;

    RegistryMsgHeader reference;
    if (participantsMsg.messageHeader != reference)
    {
        std::cout << "Received known participant message with unsupported protocol specification"
                  << std::endl;
        return;
    }

    std::cout << "Received known participant message" << std::endl;

    for (auto&& peerInfo : participantsMsg.peerInfos)
    {
        std::cout
            << "Connecting to " << peerInfo.participantName
            << " with Id " << peerInfo.participantId
            << " on " << peerInfo.acceptorHost << ":" << peerInfo.acceptorPort
            << std::endl;

        auto peer = std::make_unique<VAsioTcpPeer>(_ioContext, this);
        peer->Connect(std::move(peerInfo));

        // We connected to the other peer. tell him who we are.
        SendParticipantAnnoucement(peer.get());

        AddPeer(std::move(peer));
    }
}

void VAsioConnection::StartIoWorker()
{
    _ioWorker = std::thread{[this]() {
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
    }};
}

void VAsioConnection::AcceptConnectionsOn(asio::ip::tcp::endpoint endpoint)
{
    assert(!_tcpAcceptor);

    try
    {
        _tcpAcceptor = std::make_unique<asio::ip::tcp::acceptor>(_ioContext, endpoint);
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR: VAsioConnection failed to listening on " << endpoint << ": " << e.what() << std::endl;
        throw e;
    }

    std::cout << "INFO: VAsioConnection is listening on " << _tcpAcceptor->local_endpoint() << std::endl;

    AcceptNextConnection(*_tcpAcceptor);
}

void VAsioConnection::AcceptNextConnection(asio::ip::tcp::acceptor& acceptor)
{
    std::shared_ptr<VAsioTcpPeer> newConnection;
    try
    {
        newConnection = std::make_shared<VAsioTcpPeer>(acceptor.get_executor().context(), this);
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR: VAsioConnection cannot create listener socket!" << std::endl;
        throw e;
    }

    acceptor.async_accept(newConnection->Socket(),
        [this, newConnection, &acceptor](const asio::error_code& error) mutable
        {
            if (!error)
            {
                std::cerr << "INFO: New connection from " << newConnection->Socket().remote_endpoint() << std::endl;
                AddPeer(std::move(newConnection));
            }
            AcceptNextConnection(acceptor);
        }
    );
}

void VAsioConnection::AddPeer(std::shared_ptr<VAsioTcpPeer> newPeer)
{
    for (auto&& localReceiver : _vasioReceivers)
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
        << RegistryMessageKind::SubscriptionSent;
    for (auto&& peer : _peers)
    {
        peer->SendIbMsg(buffer);
    }
}

void VAsioConnection::RegisterNewPeerCallback(std::function<void()> callback)
{
    ExecuteOnIoThread([this, callback{std::move(callback)}]{
        _newPeerCallback = std::move(callback);
    });
}

void VAsioConnection::RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> callback)
{
    ExecuteOnIoThread([this, callback{std::move(callback)}]{
        _peerShutdownCallbacks.emplace_back(std::move(callback));
    });
}

void VAsioConnection::OnPeerShutdown(IVAsioPeer* peer)
{
    for (auto&& callback : _peerShutdownCallbacks)
    {
        callback(peer);
    }
}

void VAsioConnection::OnSocketData(IVAsioPeer* from, MessageBuffer&& buffer)
{
    VAsioMsgKind msgKind;
    buffer >> msgKind;
    switch (msgKind)
    {
    case VAsioMsgKind::Invalid:
        std::cerr << "WARNING: Received message with VAsioMsgKind::Invalid\n";
        break;
    case VAsioMsgKind::AnnounceSubscription:
        return ReceiveSubscriptionAnnouncement(from, std::move(buffer));
    case VAsioMsgKind::IbMwMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    case VAsioMsgKind::IbSimMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    case VAsioMsgKind::IbRegistryMessage:
        return ReceiveRegistryMessage(from, std::move(buffer));
    }
}

void VAsioConnection::ReceiveSubscriptionAnnouncement(IVAsioPeer* from, MessageBuffer&& buffer)
{
    VAsioMsgSubscriber subscriber;
    buffer >> subscriber;

    bool wasAdded = false;

    tt::for_each(_ibLinks, [&](auto&& linkMap) {

        using LinkType = typename std::decay_t<decltype(linkMap)>::mapped_type::element_type;

        if (subscriber.msgTypeName != LinkType::MsgTypeName())
            return;

        auto& ibLink = linkMap[subscriber.linkName];
        if (!ibLink)
        {
            ibLink = std::make_shared<LinkType>(subscriber.linkName);
        }

        ibLink->AddRemoteReceiver(from, subscriber.receiverIdx);

        wasAdded = true;

    });


    if (wasAdded)
        std::cout << "INFO: Registered subscriber for: [" << subscriber.linkName << "] " << subscriber.msgTypeName << "\n";
    else
        std::cout << "WARNING: Cannot register subscriber for: [" << subscriber.linkName << "] " << subscriber.msgTypeName << "\n";
}

void VAsioConnection::ReceiveRawIbMessage(MessageBuffer&& buffer)
{
    uint16_t receiverIdx;
    buffer >> receiverIdx;
    if (receiverIdx >= _vasioReceivers.size())
    {
        std::cerr << "WARNING: Ignoring RawIbMessage for unknown receiverIdx=" << receiverIdx << "\n";
        return;
    }
    _vasioReceivers[receiverIdx]->ReceiveRawMsg(std::move(buffer));
}

void VAsioConnection::RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback)
{
    _participantAnnouncementReceivers.emplace_back(std::move(callback));
}

void VAsioConnection::ReceiveRegistryMessage(IVAsioPeer* from, MessageBuffer&& buffer)
{
    RegistryMessageKind kind;
    buffer >> kind;
    switch (kind)
    {
    case RegistryMessageKind::Invalid:
        std::cerr << "WARNING: Received message with RegistryMessageKind::Invalid\n";
        return;
    case RegistryMessageKind::ParticipantAnnouncement:
        return ReceiveParticipantAnnouncement(from, std::move(buffer));
    case RegistryMessageKind::KnownParticipants:
        return ReceiveKnownParticpants(std::move(buffer));
    case RegistryMessageKind::SubscriptionSent:
        return ReceiveSubscriptionSentEvent();
    }
}

} // namespace mw
} // namespace ib
