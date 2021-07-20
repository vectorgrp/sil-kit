// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <array>
#include <functional>

#include "ib/mw/logging/ILogger.hpp"

#include "VAsioTcpPeer.hpp"
#include "Filesystem.hpp"
#include "Uri.hpp"



using namespace std::chrono_literals;
namespace fs = ib::filesystem;

namespace {
//only TCP/IP need platform tweaks
template<typename AcceptorT>
void SetPlatformOptions(AcceptorT&)
{
}
// local domain sockets on my WSL (Linux) require read/write permission for user
template<typename EndpointT>
void SetSocketPermissions(const EndpointT&)
{
}

template<typename AcceptorT>
void SetListenOptions(ib::mw::logging::ILogger*,
    AcceptorT&)
{
}

// platform specific definitions of utilities
#if defined(_WIN32)
#   include <mstcpip.h>
template<>
void SetPlatformOptions(asio::ip::tcp::acceptor& acceptor)
{
    using exclusive_addruse = asio::detail::socket_option::boolean<ASIO_OS_DEF(SOL_SOCKET), SO_EXCLUSIVEADDRUSE>;
    acceptor.set_option(exclusive_addruse{true});
}

template<>
void SetListenOptions(ib::mw::logging::ILogger* logger,
    asio::ip::tcp::acceptor& acceptor)
{
    // This should improve loopback performance, and have no effect on remote TCP/IP
    int enabled = 1;
    DWORD numberOfBytes = 0;
    auto result = WSAIoctl(acceptor.native_handle(),
        SIO_LOOPBACK_FAST_PATH,
        &enabled,
        sizeof(enabled),
        nullptr,
        0,
        &numberOfBytes,
        0,
        0);

    if (result == SOCKET_ERROR)
    {
        auto lastError = ::GetLastError();
        logger->Warn("VAsioConnection: Setting Loopback FastPath failed: WSA IOCtl last error: {}", lastError);
    }
}
#else

template<>
void SetPlatformOptions(asio::ip::tcp::acceptor& acceptor)
{
    // We enable the SO_REUSEADDR flag on POSIX, this allows reusing a socket's address more quickly.
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address{true});
}

template<>
void SetSocketPermissions(const asio::local::stream_protocol::endpoint& endpoint)
{
    const auto path = endpoint.path();
    (void)chmod(path.c_str(), 0770);
}

template<>
void SetListenOptions(ib::mw::logging::ILogger* ,
    asio::ip::tcp::acceptor& )
{
    // no op
}

#endif

//!< Note that local ipc (unix domain) sockets have a path limit (108 characters, typically)
// Using the current working directory as part of a domain socket path might result in 
// a runtime exception. We create a unique temporary file path, with a fixed length.
auto makeLocalEndpoint(const std::string& participantName, const ib::mw::ParticipantId& id, const int domainId) -> asio::local::stream_protocol::endpoint
{
    asio::local::stream_protocol::endpoint result;
    const auto bounded_name = participantName.substr(0, 
        std::min<size_t>(participantName.size(), 10));

    // We hash the participant name, ID and the current working directory
    // as part of the temporary file name, so we can have multiple local simulations
    // started from different working directories, but a shared temporary directory.
    // NB keep the variable part as short as possible.

    const auto unique_id = std::hash<std::string>{}(participantName
        + std::to_string(id)
        + std::to_string(domainId)
        + fs::current_path().string()
    );

    std::stringstream path;
    path << fs::temp_directory_path().string()
        << fs::path::preferred_separator
        << bounded_name
        << std::hex << unique_id
        << ".vib";

    result.path(path.str());
    return result;
}

//Debug  print of given peer infos
auto printUris(const ib::mw::VAsioPeerUri& info) -> std::string 
{
    std::stringstream ss;
    for (auto& uri : info.acceptorUris)
    {
        ss << uri << ", ";
    }
    return ss.str();
}

} //anonymous namespace



namespace std {
inline std::ostream& operator<< (std::ostream& out,
    const asio::generic::stream_protocol::socket& sock)
{
    const auto local_family = asio::local::stream_protocol{}.family();
    const auto ep = sock.local_endpoint();
    if (ep.protocol().family() == asio::ip::tcp::v4().family()
        || ep.protocol().family() == asio::ip::tcp::v6().family())
    {
        //we have an actual remote end
        auto remote_ep = sock.remote_endpoint();
        out << reinterpret_cast<const asio::ip::tcp::endpoint& >(remote_ep);
    }
    else if (ep.protocol().family() == local_family)
    {
        // The underlying sockaddr_un contains the path, zero terminated.
        const auto* data = static_cast<const char*>(ep.data()->sa_data);
        out << '"' << data << '"';
    }
    else
    {
        out << "Unknown Endpoint family=" << ep.protocol().family();
    }
    return out;
}
} //end namespace std 

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

    auto&& localAcceptor = GetAcceptor<asio::local::stream_protocol::acceptor>();
    if (localAcceptor)
    {
        //clean up local ipc sockets
        (void)fs::remove(localAcceptor->local_endpoint().path());
    }

}

void VAsioConnection::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

void VAsioConnection::JoinDomain(uint32_t domainId)
{
    assert(_logger);

    if (_config.middlewareConfig.vasio.enableDomainSockets)
    {
        // We pick a random file name for local domain sockets
        try
        {
            AcceptLocalConnections(domainId);
        }
        catch (const std::exception& ex)
        {
            _logger->Warn("VasioConnection::JoinDomain: Cannot accept local IPC connections: {}, pwd={}",
                ex.what(), fs::current_path().string());
        }
    }
    // We let the operating system choose a free TCP port
    AcceptConnectionsOn(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 0));

    auto& vasioConfig = _config.middlewareConfig.vasio;

    VAsioPeerUri registryUri;
    registryUri.participantName = "IbRegistry";
    registryUri.participantId = 0;

    //setup local acceptor URI
    auto registryLocalEndpoint = makeLocalEndpoint(registryUri.participantName,
        registryUri.participantId, domainId);
    const auto localUri = Uri{registryLocalEndpoint}.EncodedString();
    registryUri.acceptorUris.push_back(localUri);

    _logger->Debug("Connecting to VAsio registry");

    auto registry = std::make_unique<VAsioTcpPeer>(_ioContext.get_executor(), this, _logger);
    bool ok = false;

    // NB: We attempt to connect multiple times. The registry might be a separate process
    //     which may still be initializing when we are running. For example, this happens when all
    //     participants are started in a shell, and the registry is started in the background.

    auto multipleConnectAttempts = [&ok, &registry, &vasioConfig](auto registryUri) {
        for (auto i = 0; i < vasioConfig.registry.connectAttempts; i++)
        {
            try
            {
                registry->Connect(registryUri);
                ok = true;
                break;
            }
            catch (const std::exception&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
            }
        }
    };

    // First, attempt local connections if available:
    multipleConnectAttempts(registryUri);

    // Fall back to TCP connections:
    if (!ok)
    {
        registryUri.acceptorUris.clear();
        auto registryPort = static_cast<uint16_t>(vasioConfig.registry.port + domainId);
        registryUri.acceptorUris.push_back(
            Uri{ vasioConfig.registry.hostname,  registryPort }.EncodedString()
        );
        multipleConnectAttempts(registryUri);

    }
    // Neither local nor tcp is working.
    if (!ok)
    {
        //re-add local URI for debugging purposes:
        registryUri.acceptorUris.push_back(localUri);

        _logger->Error("Failed to connect to VAsio registry (number of attempts: {})",
            vasioConfig.registry.connectAttempts);
        _logger->Info("   Make sure that the IbRegistry is up and running and is listening on the following URIs: {}.",
            printUris(registryUri));
        _logger->Info("   If a registry is unable to open a listening socket it will only be reachable via local domain sockets, which depend on the working directory.");
        _logger->Info("   Make sure that the hostname can be resolved and is reachable.");
        _logger->Info("   You can configure the IbRegistry hostname and port via the IbConfig.");
        _logger->Info("   The IbRegistry executable can be found in your IB installation folder:");
        _logger->Info("     INSTALL_DIR/bin/IbRegistry[.exe]");
        throw std::runtime_error{"ERROR: Failed to connect to VAsio registry"};
    }

    _logger->Info("Connected to registry {}", printUris(registry->GetUri()));
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

    const RegistryMsgHeader reference{};

    if (announcement.messageHeader != reference)
    {
        _logger->Warn("Received participant announcement message with unsupported protocol specification.");
        return;
    }

    _logger->Debug("Received participant announcement from {}", announcement.peerInfo.participantName);

    from->SetInfo(announcement.peerInfo);
    from->SetUri(announcement.peerUri);
    for (auto&& receiver : _participantAnnouncementReceivers)
    {
        receiver(from, announcement);
    }
    SendParticipantAnnoucementReply(from);
}

void VAsioConnection::SendParticipantAnnoucement(IVAsioPeer* peer)
{
    //Legacy Info for interop
    VAsioPeerInfo localInfo{ _participantName, _participantId };
    //URI encoded infos
    VAsioPeerUri uri{ _participantName, _participantId };

    //Ensure that the local acceptor is the first entry in the acceptorUris
    auto&& localAcceptor = GetAcceptor<asio::local::stream_protocol::acceptor>();
    if (localAcceptor)
    {
        uri.acceptorUris.push_back(
            Uri{ localAcceptor->local_endpoint() }.EncodedString()
        );
    }
    auto&& tcpAcceptor = GetAcceptor<asio::ip::tcp::acceptor>();
    if (!tcpAcceptor)
    {
        throw std::runtime_error{ "VasioConnection: cannot send announcement on TCP: tcp-acceptor is missing" };
    }

    auto epUri = Uri{tcpAcceptor->local_endpoint()};
    localInfo.acceptorHost = epUri.Host();
    localInfo.acceptorPort = epUri.Port();
    uri.acceptorUris.emplace_back(epUri.EncodedString());

    ParticipantAnnouncement announcement;
    announcement.peerInfo = std::move(localInfo);
    announcement.peerUri = std::move(uri);

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

    const RegistryMsgHeader reference{};
    if (participantsMsg.messageHeader != reference)
    {
        _logger->Warn("Received known participant message with unsupported protocol specification");
        return;
    }

    _logger->Debug("Received known participants list from IbRegistry");

    auto connectPeer = [this](const auto peerUri) {
        _logger->Debug("Connecting to {} with Id {} on {}",
            peerUri.participantName,
            peerUri.participantId,
            printUris(peerUri));

        auto peer = std::make_unique<VAsioTcpPeer>(_ioContext.get_executor(), this, _logger);
        try
        {
            peer->Connect(std::move(peerUri));
        }
        catch (const std::exception&)
        {
            return;
        }

        // We connected to the other peer. tell him who we are.
        _pendingParticipantReplies.push_back(peer.get());
        SendParticipantAnnoucement(peer.get());

        AddPeer(std::move(peer));
    };
    // check URI first
    if (!participantsMsg.peerUris.empty())
    {
        for (auto&& uri : participantsMsg.peerUris)
        {
            connectPeer(uri);
        }
    }
    else
    {
        // interop with older VIB participants:
        for (auto&& peerInfo : participantsMsg.peerInfos)
        {
            VAsioPeerUri uri{ peerInfo.participantName, peerInfo.participantId };
            uri.acceptorUris.push_back(
                Uri{ peerInfo.acceptorHost, peerInfo.acceptorPort }.EncodedString()
            );
            connectPeer(uri);
        }
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

void VAsioConnection::AcceptLocalConnections(uint32_t domainId)
{
    auto localEndpoint = makeLocalEndpoint(_participantName, _participantId, domainId);

    //file must not exist before we bind/listen on it
    (void)fs::remove(localEndpoint.path());

    AcceptConnectionsOn(localEndpoint);
}

template<typename EndpointT>
void VAsioConnection::AcceptConnectionsOn(EndpointT endpoint)
{
    using AcceptorT = typename EndpointT::protocol_type::acceptor;
    auto&& acceptor = std::get<std::unique_ptr<AcceptorT>>(_acceptors);
    if (acceptor)
    {
        // we already have an acceptor for the given endpoint type
        std::stringstream endpointName;
        endpointName << endpoint;
        throw std::logic_error{ "VAsioConnection: acceptor already open for endpoint type: " 
            + endpointName.str()};
    }

    try
    {
        acceptor = std::make_unique<AcceptorT>(_ioContext);
        acceptor->open(endpoint.protocol());
        SetPlatformOptions(*acceptor);
        acceptor->bind(endpoint);
        SetSocketPermissions(endpoint);
        acceptor->listen();
        SetListenOptions(_logger, *acceptor);
    }
    catch (const std::exception& e)
    {
        _logger->Error("VAsioConnection failed to listening on {}: {}", endpoint, e.what());
        throw;
    }

    _logger->Debug("VAsioConnection is listening on {}", acceptor->local_endpoint());

    AcceptNextConnection(*acceptor);
}

template<typename AcceptorT>
void VAsioConnection::AcceptNextConnection(AcceptorT& acceptor)
{
    std::shared_ptr<VAsioTcpPeer> newConnection;
    try
    {
        newConnection = std::make_shared<VAsioTcpPeer>(_ioContext.get_executor(), this, _logger);
    }
    catch (const std::exception& e)
    {
        _logger->Error("VAsioConnection cannot create listener socket: {}", e.what());
        throw;
    }

    acceptor.async_accept(newConnection->Socket(),
        [this, newConnection, &acceptor](const asio::error_code& error) mutable
        {
            if (!error)
            {
                _logger->Debug("New connection from {}", newConnection->Socket());
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

void VAsioConnection::NotifyShutdown()
{
    _isShuttingDown = true;
}

void VAsioConnection::UpdateParticipantStatusOnConnectionLoss(IVAsioPeer* peer)
{
    if (_isShuttingDown)
    {
        _logger->Debug("Ignoring UpdateParticipantStatusOnConnectionLoss because we're shutting down");
        return;
    }

    auto& info = peer->GetInfo();

    EndpointAddress address{info.participantId, 1024};

    ib::mw::sync::ParticipantStatus msg;
    msg.participantName = info.participantName;
    msg.state = ib::mw::sync::ParticipantState::Error;
    msg.enterReason = "Connection Lost";
    msg.enterTime = std::chrono::system_clock::now();
    msg.refreshTime = std::chrono::system_clock::now();

    auto&& link = GetLinkByName<ib::mw::sync::ParticipantStatus>("default");
    link->DistributeRemoteIbMessage(std::move(address), msg);

    _logger->Error("Lost connection to participant {}", info.participantName);
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
