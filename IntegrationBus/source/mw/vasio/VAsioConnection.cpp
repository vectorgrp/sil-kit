// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

#include "ib/cfg/string_utils.hpp"

#include "VAsioTcpPeer.hpp"

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
    RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { UpdateParticipantStatusOnConnectionLoss(peer); });
}

VAsioConnection::~VAsioConnection()
{
    if (_ioWorker.joinable())
    {
        _ioContext.stop();
        _ioWorker.join();
    }
}

void VAsioConnection::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

void VAsioConnection::JoinDomain(uint32_t domainId)
{
    assert(_logger);

    // We let the operating system choose a free port
    AcceptConnectionsOn(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 0));

    auto& vasioConfig = _config.middlewareConfig.vasio;

    VAsioPeerInfo registryInfo;
    registryInfo.participantName = "IbRegistry";
    registryInfo.participantId = 0;
    registryInfo.acceptorHost = vasioConfig.registry.hostname;
    registryInfo.acceptorPort = static_cast<uint16_t>(vasioConfig.registry.port + domainId);

    _logger->Debug("Connecting to VAsio registry");

    auto registry = std::make_unique<VAsioTcpPeer>(_ioContext, this, _logger);

    try
    {
        registry->Connect(registryInfo);
    }
    catch (const std::exception&)
    {
        _logger->Error("Failed to connect to VAsio registry");
        _logger->Info("   Make sure that the IbRegistry is up and running and is listening on port {}.", registryInfo.acceptorPort);
        _logger->Info("   Make sure that the hostname \"{}\" can be resolved and is reachable.", registryInfo.acceptorHost);
        _logger->Info("   You can configure the IbRegistry hostname and port via the IbConfig.");
        _logger->Info("   The IbRegistry executable can be found in your IB installation folder:");
        _logger->Info("     INSTALL_DIR/bin/IbRegistry[.exe]");

        throw std::runtime_error{"ERROR: Failed to connect to VAsio registry"};
    }

    registry->StartAsyncRead();

    SendParticipantAnnoucement(registry.get());
    _registry = std::move(registry);
    
    StartIoWorker();

    auto receivedAllReplies = _receivedAllParticipantReplies.get_future();
    _logger->Debug("VAsio is waiting for known participants list from registry.");
    receivedAllReplies.wait();
    _logger->Trace("VAsio received announcement replies from all participants.");
}

void VAsioConnection::ReceiveParticipantAnnouncement(IVAsioPeer* from, MessageBuffer&& buffer)
{
    ParticipantAnnouncement announcement;
    buffer >> announcement;

    RegistryMsgHeader reference;
    if (announcement.messageHeader != reference)
    {
        _logger->Warn("Received participant announcement message with unsupported protocol specification");
        return;
    }

    _logger->Debug("Received participant announcement from {}", announcement.peerInfo.participantName);

    from->SetInfo(announcement.peerInfo);
    for (auto&& receiver : _participantAnnouncementReceivers)
    {
        receiver(from, announcement);
    }
    SendParticipantAnnoucementReply(from);
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

    _logger->Debug("Sending participant announcement to {}", peer->GetInfo().participantName);
    peer->SendIbMsg(std::move(buffer));
}

void VAsioConnection::ReceiveParticipantAnnouncementReply(IVAsioPeer* from, MessageBuffer&& buffer)
{
    ParticipantAnnouncementReply reply;
    buffer >> reply;

    for (auto& subscriber : reply.subscribers)
    {
        TryAddRemoteSubscriber(from, subscriber);
    }

    _logger->Debug("Received participant announcement reply from {}", from->GetInfo().participantName);

    auto iter = std::find(
        _pendingParticipantReplies.begin(),
        _pendingParticipantReplies.end(),
        from);
    if (iter != _pendingParticipantReplies.end())
    {
        _pendingParticipantReplies.erase(iter);
        if (_pendingParticipantReplies.empty())
        {
            _receivedAllParticipantReplies.set_value();
        }
    }
}

void VAsioConnection::SendParticipantAnnoucementReply(IVAsioPeer* peer)
{
    ParticipantAnnouncementReply reply;
    std::transform(_vasioReceivers.begin(), _vasioReceivers.end(), std::back_inserter(reply.subscribers),
                   [](const auto& subscriber) { return subscriber->GetDescriptor(); });

    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncementReply
           << reply;

    _logger->Debug("Sending participant announcement reply to {}", peer->GetInfo().participantName);
    peer->SendIbMsg(std::move(buffer));
}

void VAsioConnection::ReceiveKnownParticpants(MessageBuffer&& buffer)
{
    KnownParticipants participantsMsg;
    buffer >> participantsMsg;

    RegistryMsgHeader reference;
    if (participantsMsg.messageHeader != reference)
    {
        _logger->Warn("Received known participant message with unsupported protocol specification");
        return;
    }

    _logger->Debug("Received known participants list from IbRegistry");

    for (auto&& peerInfo : participantsMsg.peerInfos)
    {
        _logger->Debug("Connecting to {} with Id {} on {}:{}",
            peerInfo.participantName,
            peerInfo.participantId,
            peerInfo.acceptorHost,
            peerInfo.acceptorPort);

        auto peer = std::make_unique<VAsioTcpPeer>(_ioContext, this, _logger);
        try
        {
            peer->Connect(std::move(peerInfo));
        }
        catch (const std::exception&)
        {
            continue;
        }

        // We connected to the other peer. tell him who we are.
        _pendingParticipantReplies.push_back(peer.get());
        SendParticipantAnnoucement(peer.get());

        AddPeer(std::move(peer));
    }
    if (_pendingParticipantReplies.empty())
    {
        _receivedAllParticipantReplies.set_value();
    }
    _logger->Trace("VAsio is waiting for {} ParticipantAnnouncementReplies", _pendingParticipantReplies.size());
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
            _logger->Error("Something went wrong: {}", error.what());
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
        _logger->Error("VAsioConnection failed to listening on {}: {}", endpoint, e.what());
        throw e;
    }

    _logger->Debug("VAsioConnection is listening on {}", _tcpAcceptor->local_endpoint());

    AcceptNextConnection(*_tcpAcceptor);
}

void VAsioConnection::AcceptNextConnection(asio::ip::tcp::acceptor& acceptor)
{
    std::shared_ptr<VAsioTcpPeer> newConnection;
    try
    {
        newConnection = std::make_shared<VAsioTcpPeer>(acceptor.get_executor().context(), this, _logger);
    }
    catch (std::exception& e)
    {
        _logger->Error("VAsioConnection cannot create listener socket");
        throw e;
    }

    acceptor.async_accept(newConnection->Socket(),
        [this, newConnection, &acceptor](const asio::error_code& error) mutable
        {
            if (!error)
            {
                _logger->Debug("New connection from {}", newConnection->Socket().remote_endpoint());
                AddPeer(std::move(newConnection));
            }
            AcceptNextConnection(acceptor);
        }
    );
}

void VAsioConnection::AddPeer(std::shared_ptr<VAsioTcpPeer> newPeer)
{
    newPeer->StartAsyncRead();

    _peers.emplace_back(std::move(newPeer));   
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

void VAsioConnection::UpdateParticipantStatusOnConnectionLoss(IVAsioPeer* peer)
{
    auto& info = peer->GetInfo();

    EndpointAddress address{info.participantId, 1024};

    ib::mw::sync::ParticipantStatus msg;
    msg.participantName = info.participantName;
    msg.state = ib::mw::sync::ParticipantState::Error;
    msg.enterReason = "Shutdown";
    msg.enterTime = std::chrono::system_clock::now();
    msg.refreshTime = std::chrono::system_clock::now();

    auto&& link = GetLinkByName<ib::mw::sync::ParticipantStatus>("default");
    link->DistributeRemoteIbMessage(std::move(address), msg);
}

void VAsioConnection::OnSocketData(IVAsioPeer* from, MessageBuffer&& buffer)
{
    VAsioMsgKind msgKind;
    buffer >> msgKind;
    switch (msgKind)
    {
    case VAsioMsgKind::Invalid:
        _logger->Warn("Received message with VAsioMsgKind::Invalid");
        break;
    case VAsioMsgKind::SubscriptionAnnouncement:
        return ReceiveSubscriptionAnnouncement(from, std::move(buffer));
    case VAsioMsgKind::SubscriptionAcknowledge:
        return ReceiveSubscriptionAcknowledge(from, std::move(buffer));
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
    bool wasAdded = TryAddRemoteSubscriber(from, subscriber);

    // send acknowledge
    SubscriptionAcknowledge ack;
    ack.subscriber = std::move(subscriber);
    ack.status = wasAdded
        ? SubscriptionAcknowledge::Status::Success
        : SubscriptionAcknowledge::Status::Failed;

    MessageBuffer ackBuffer;
    uint32_t msgSizePlaceholder{0u};
    ackBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::SubscriptionAcknowledge
        << ack;

    from->SendIbMsg(std::move(ackBuffer));
}

void VAsioConnection::ReceiveSubscriptionAcknowledge(IVAsioPeer* from, MessageBuffer&& buffer)
{
    SubscriptionAcknowledge ack;
    buffer >> ack;

    if (ack.status != SubscriptionAcknowledge::Status::Success)
    {
        _logger->Error("Failed to subscribe [{}] {} from {}"
            , ack.subscriber.linkName
            , ack.subscriber.msgTypeName
            , from->GetInfo().participantName);
    }

    // we remove the pending subscription in any case. As there will not follow a new, successful acknowledge
    auto iter = std::find(_pendingSubscriptionAcknowledges.begin(),
                          _pendingSubscriptionAcknowledges.end(),
                          std::make_pair(from, ack.subscriber));
    if (iter != _pendingSubscriptionAcknowledges.end())
    {
        _pendingSubscriptionAcknowledges.erase(iter);
        if (_pendingSubscriptionAcknowledges.empty())
        {
            _receivedAllSubscriptionAcknowledges.set_value();
        }
    }
}

bool VAsioConnection::TryAddRemoteSubscriber(IVAsioPeer* from, const VAsioMsgSubscriber& subscriber)
{
    bool wasAdded = false;

    tt::for_each(_ibLinks, [&](auto&& linkMap) {

        using LinkType = typename std::decay_t<decltype(linkMap)>::mapped_type::element_type;

        if (subscriber.msgTypeName != LinkType::MsgTypeName())
            return;

        auto& ibLink = linkMap[subscriber.linkName];
        if (!ibLink)
        {
            ibLink = std::make_shared<LinkType>(subscriber.linkName, _logger);
        }

        ibLink->AddRemoteReceiver(from, subscriber.receiverIdx);

        wasAdded = true;

    });


    if (wasAdded)
        _logger->Debug("Registered subscription for [{}] {} from {}", subscriber.linkName, subscriber.msgTypeName, from->GetInfo().participantName);
    else
        _logger->Warn("Cannot register subscription for [{}] {} from {}", subscriber.linkName, subscriber.msgTypeName, from->GetInfo().participantName);

    return wasAdded;
}

void VAsioConnection::ReceiveRawIbMessage(MessageBuffer&& buffer)
{
    uint16_t receiverIdx;
    buffer >> receiverIdx;
    if (receiverIdx >= _vasioReceivers.size())
    {
        _logger->Warn("Ignoring RawIbMessage for unknown receiverIdx={}", receiverIdx);
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
        _logger->Warn("Received message with RegistryMessageKind::Invalid");
        return;
    case RegistryMessageKind::ParticipantAnnouncement:
        return ReceiveParticipantAnnouncement(from, std::move(buffer));
    case RegistryMessageKind::ParticipantAnnouncementReply:
        return ReceiveParticipantAnnouncementReply(from, std::move(buffer));
    case RegistryMessageKind::KnownParticipants:
        return ReceiveKnownParticpants(std::move(buffer));
    }
}

} // namespace mw
} // namespace ib
