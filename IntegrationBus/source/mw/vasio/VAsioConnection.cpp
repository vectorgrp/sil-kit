// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"
#include "VAsioProtocolVersion.hpp"

#include "SerializedMessage.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <array>
#include <functional>
#include <cctype>

#include "ib/mw/logging/ILogger.hpp"

#include "VAsioTcpPeer.hpp"
#include "Filesystem.hpp"
#include "SetThreadName.hpp"
#include "Uri.hpp"

using namespace std::chrono_literals;
namespace fs = ib::filesystem;

namespace {

// only TCP/IP need platform tweaks
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

#   if !defined(__MINGW32__)
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
#   endif //__MINGW32__
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

auto printableName(const std::string& participantName) -> std::string
{
    std::string safeName;
    for (const auto& ch : participantName)
    {
        if (std::isalnum(ch))
        {
            safeName.push_back(ch);
        }
        else
        {
            safeName += fmt::format("{:x}", static_cast<unsigned char>(ch));
        }
    }
    return safeName;
}

//!< Note that local ipc (unix domain) sockets have a path limit (108 characters, typically)
// Using the current working directory as part of a domain socket path might result in 
// a runtime exception. We create a unique temporary file path, with a fixed length.
auto makeLocalEndpoint(const std::string& participantName, const ib::mw::ParticipantId& id, const int domainId) -> asio::local::stream_protocol::endpoint
{
    asio::local::stream_protocol::endpoint result;
    // Ensure the participantName is in a useful encoding
    const auto safe_name = printableName(participantName);
    const auto bounded_name = safe_name.substr(0,
        std::min<size_t>(safe_name.size(), 10));

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
auto printUris(const ib::mw::VAsioPeerInfo& info) -> std::string 
{
    std::stringstream ss;
    for (auto& uri : info.acceptorUris)
    {
        ss << uri << ", ";
    }
    return ss.str();
}

auto fromAsioEndpoint(const asio::local::stream_protocol::endpoint& ep)
{
    std::stringstream uri;
    const std::string localPrefix{ "local://" };
    uri << localPrefix << ep.path();
    return ib::mw::Uri::Parse(uri.str());
}

auto fromAsioEndpoint(const asio::ip::tcp::endpoint& ep)
{
    std::stringstream uri;
    const std::string tcpPrefix{ "tcp://" };
    uri << tcpPrefix << ep; //will be "ipv4:port" or "[ipv6]:port"
    return ib::mw::Uri::Parse(uri.str());
}

} // namespace

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

VAsioConnection::VAsioConnection(ib::cfg::ParticipantConfiguration config,
    std::string participantName, ParticipantId participantId,
    ProtocolVersion version)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
    , _tcp4Acceptor{_ioContext}
    , _tcp6Acceptor{_ioContext}
    , _localAcceptor{_ioContext}
    , _version{version}
{
    RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { UpdateParticipantStatusOnConnectionLoss(peer); });
    _hashToParticipantName.insert(std::pair<uint64_t, std::string>(ib::util::hash::Hash(_participantName), _participantName));
}

VAsioConnection::~VAsioConnection()
{
    if (_ioWorker.joinable())
    {
        _ioContext.stop();
        _ioWorker.join();
    }

    if (_localAcceptor.is_open())
    {
        //clean up local ipc sockets
        (void)fs::remove(_localAcceptor.local_endpoint().path());
    }

}

void VAsioConnection::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

void VAsioConnection::JoinDomain(uint32_t domainId)
{
    assert(_logger);

    if (_config.middleware.enableDomainSockets)
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
    // The address will be substituted by the registry, from the actual connection endpoint's address.
    AcceptConnectionsOn(_tcp4Acceptor, tcp::endpoint{asio::ip::tcp::v4(), 0});
    AcceptConnectionsOn(_tcp6Acceptor, tcp::endpoint{asio::ip::tcp::v6(), 0});

    auto registry = std::make_unique<VAsioTcpPeer>(_ioContext.get_executor(), this, _logger);
    bool ok = false;

    // NB: We attempt to connect multiple times. The registry might be a separate process
    //     which may still be initializing when we are running. For example, this happens when all
    //     participants are started in a shell, and the registry is started in the background.
    auto registryCfg = _config.middleware.registry;
    auto multipleConnectAttempts = [ &registry, &registryCfg](const auto& registryUriRef) {
        for (auto i = 0; i < registryCfg.connectAttempts; i++)
        {
            try
            {
                registry->Connect(registryUriRef);
                return true;
            }
            catch (const std::exception&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
            }
        }
        return false;
    };

    // Compute a list of Registry URIs and attempt to connect as per config
    VAsioPeerInfo registryUri;
    registryUri.participantName = "IbRegistry";
    registryUri.participantId = 0;

    // setup local acceptor URI (we can infer this based on the IbRegistry's name)
    auto registryLocalEndpoint = makeLocalEndpoint(registryUri.participantName,
        registryUri.participantId, domainId);
    const auto localUri = fromAsioEndpoint(registryLocalEndpoint).EncodedString();
    registryUri.acceptorUris.push_back(localUri);

    _logger->Debug("Connecting to VAsio registry");


    // First, attempt local connections if available:
    if (_config.middleware.enableDomainSockets)
    {
        ok = multipleConnectAttempts(registryUri);
    }

    // Fall back to TCP connections:
    if (!ok)
    {
        // setup TCP remote URI
        registryUri.acceptorUris.clear();
        auto registryPort = static_cast<uint16_t>(registryCfg.port + domainId);
        registryUri.acceptorUris.push_back(
            Uri{ registryCfg.hostname,  registryPort }.EncodedString()
        );
        ok = multipleConnectAttempts(registryUri);

    }
    // Neither local nor tcp is working.
    if (!ok)
    {
        if (_config.middleware.enableDomainSockets)
        {
            // re-add local URI for user info output:
            registryUri.acceptorUris.push_back(localUri);
        }

        _logger->Error("Failed to connect to VAsio registry (number of attempts: {})",
                       _config.middleware.registry.connectAttempts);
        _logger->Info("   Make sure that the IbRegistry is up and running and is listening on the following URIs: {}.",
            printUris(registryUri));
        _logger->Info("   If a registry is unable to open a listening socket it will only be reachable"
                      " via local domain sockets, which depend on the working directory"
                      " and the middleware configuration ('enableDomainSockets').");
        _logger->Info("   Make sure that the hostname can be resolved and is reachable.");
        _logger->Info("   You can configure the IbRegistry hostname and port via the IbConfig.");
        _logger->Info("   The IbRegistry executable can be found in your IB installation folder:");
        _logger->Info("     INSTALL_DIR/bin/IbRegistry[.exe]");
        throw std::runtime_error{"ERROR: Failed to connect to VAsio registry"};
    }

    _logger->Info("Connected to registry {}", printUris(registry->GetInfo()));
    registry->StartAsyncRead();

    SendParticipantAnnouncement(registry.get());
    _registry = std::move(registry);

    StartIoWorker();

    auto receivedAllReplies = _receivedAllParticipantReplies.get_future();
    _logger->Debug("VAsio is waiting for known participants list from registry.");

    auto waitOk = receivedAllReplies.wait_for(5s);
    if(waitOk == std::future_status::timeout)
    {
        _logger->Error("VIB timeout during connection handshake with IbRegistry.");
        throw ProtocolError("VIB timeout during connection handshake with IbRegistry.");
    }
    // check if an exception was set:
    receivedAllReplies.get();

    _logger->Trace("VAsio received announcement replies from all participants.");
}

void VAsioConnection::NotifyNetworkIncompatibility(const RegistryMsgHeader& other,
                                                   const std::string& otherParticipantName)
{
    const auto errorMsg = fmt::format("Network incompatibility between this version range ({})"
        " and connecting participant '{}' ({})",
        MapVersionToRelease(to_header(_version)),
        otherParticipantName,
        MapVersionToRelease(other)
    );
    _logger->Critical(errorMsg);
    std::cerr << "ERROR: " << errorMsg << std::endl;
}

void VAsioConnection::ReceiveParticipantAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer)
{
    const auto remoteHeader = buffer.GetRegistryMessageHeader();

    // check if we support the remote peer's protocol version or signal a handshake failure
    if(ProtocolVersionSupported(remoteHeader))
    {
        from->SetProtocolVersion(from_header(remoteHeader));
    }
    else
    {
        // it is not safe to decode the ParticipantAnnouncement
        NotifyNetworkIncompatibility(remoteHeader, from->GetRemoteAddress());

        ParticipantAnnouncementReply reply;
        reply.status = ParticipantAnnouncementReply::Status::Failed;
        reply.remoteHeader = to_header(_version); // tell remote peer what version we intent to speak
        from->SendIbMsg(SerializedMessage{from->GetProtocolVersion(), reply});

        return;
    }

    // after negotiating the Version, we can safely deserialize the remaining message
    // ParticipantAnnouncement announcement;
    buffer.SetProtocolVersion(from->GetProtocolVersion());
    auto announcement = buffer.Deserialize<ParticipantAnnouncement>();

    _logger->Debug("Received participant announcement from {}, protocol version {}.{}", announcement.peerInfo.participantName, announcement.messageHeader.versionHigh,
        announcement.messageHeader.versionLow);

    from->SetInfo(announcement.peerInfo);
    auto& service = dynamic_cast<IIbServiceEndpoint&>(*from);
    auto serviceDescriptor = service.GetServiceDescriptor();
    serviceDescriptor.SetParticipantName(announcement.peerInfo.participantName);
    service.SetServiceDescriptor(serviceDescriptor);
    for (auto&& receiver : _participantAnnouncementReceivers)
    {
        receiver(from, announcement);
    }
    AddParticipantToLookup(announcement.peerInfo.participantName);

    SendParticipantAnnouncementReply(from);
}

void VAsioConnection::SendParticipantAnnouncement(IVAsioPeer* peer)
{
    // Legacy Info for interop
    // URI encoded infos
    VAsioPeerInfo info{ _participantName, _participantId, {}, {/*capabilities*/}};

    // Ensure that the local acceptor is the first entry in the acceptorUris
    if (_localAcceptor.is_open())
    {
        info.acceptorUris.push_back(
            fromAsioEndpoint(_localAcceptor.local_endpoint()).EncodedString()
        );
    }

    if (!_tcp4Acceptor.is_open() && !_tcp6Acceptor.is_open())
    {
        throw std::runtime_error{ "VasioConnection: cannot send announcement on TCP: tcp-acceptors for IPv4 and IPv6 are missing" };
    }

    auto epUri = fromAsioEndpoint(_tcp4Acceptor.local_endpoint());
    info.acceptorUris.emplace_back(epUri.EncodedString());
    if (_tcp6Acceptor.is_open())
    {
        epUri = fromAsioEndpoint(_tcp6Acceptor.local_endpoint());
        info.acceptorUris.emplace_back(epUri.EncodedString());
    }


    ParticipantAnnouncement announcement;
    // We override the default protocol version here for integration testing
    announcement.messageHeader = to_header(_version);
    announcement.peerInfo = std::move(info);

    _logger->Debug("Sending participant announcement to {}", peer->GetInfo().participantName);
    peer->SendIbMsg(SerializedMessage{announcement});

}

void VAsioConnection::ReceiveParticipantAnnouncementReply(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto reply = buffer.Deserialize<ParticipantAnnouncementReply>();
    const auto& remoteVersion = from_header(reply.remoteHeader);

    if (reply.status == ParticipantAnnouncementReply::Status::Failed)
    {
        _logger->Warn("Received failed participant announcement reply from {}",
            from->GetInfo().participantName);
        const auto handshakeHeader = to_header(from->GetProtocolVersion());
        // check what went wrong during the handshake
        if(reply.remoteHeader.preambel != handshakeHeader.preambel)
        {
            auto msg = fmt::format("VIB Connection Handshake: ParticipantAnnouncementReply contains invalid preambel in header. check endianess on participant {}.", from->GetInfo().participantName);
        }
        const auto msg = fmt::format("VIB Connection Handshake: ParticipantAnnouncementReply contains unsupported version."
                        " participant={} participant-version={}",
                        from->GetInfo().participantName, remoteVersion);

        if(from->GetInfo().participantId == RegistryParticipantId)
        {
            // handshake with IbRegistry failed, we need to abort.
            auto error = ProtocolError(msg);
            _receivedAllParticipantReplies.set_exception(std::make_exception_ptr(error)); //propagate to main thread
            throw error; // for I/O thread
        }
        return;
    }

    from->SetProtocolVersion(remoteVersion);
    for (auto& subscriber : reply.subscribers)
    {
        TryAddRemoteSubscriber(from, subscriber);
    }

    _logger->Debug("Received participant announcement reply from {} protocol version {}",
        from->GetInfo().participantName, remoteVersion);

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

void VAsioConnection::SendParticipantAnnouncementReply(IVAsioPeer* peer)
{
    ParticipantAnnouncementReply reply;
    reply.status = ParticipantAnnouncementReply::Status::Success;
    std::transform(_vasioReceivers.begin(), _vasioReceivers.end(), std::back_inserter(reply.subscribers),
                   [](const auto& subscriber) { return subscriber->GetDescriptor(); });

    // tell the remote peer what *our* protocol version is that we
    // can accept for this peer
    reply.remoteHeader = to_header(peer->GetProtocolVersion());
    _logger->Debug("Sending participant announcement reply to {} with protocol version {}.{}",
        peer->GetInfo().participantName, reply.remoteHeader.versionHigh,
        reply.remoteHeader.versionLow);
    peer->SendIbMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::AddParticipantToLookup(const std::string& participantName)
{
    const auto result = _hashToParticipantName.insert({ib::util::hash::Hash(participantName), participantName});
    if (result.second == false)
    {
        _logger->Warn("Warning: Received announcement of participant '{}', which was already announced before.",
                      participantName);
    }
}

const std::string& VAsioConnection::GetParticipantFromLookup(const std::uint64_t participantId) const
{
    const auto participantIter = _hashToParticipantName.find(participantId);
    if (participantIter == _hashToParticipantName.end())
    {
        throw std::runtime_error{"VAsioConnection: could not find participant in participant cache"};
    }
    return participantIter->second;
}

void VAsioConnection::ReceiveKnownParticpants(IVAsioPeer* peer, SerializedMessage&& buffer)
{
    auto participantsMsg = buffer.Deserialize<KnownParticipants>();

    // After receiving a ParticipantAnnouncement the Registry will send a KnownParticipants message
    // check if we support its version here
    if(ProtocolVersionSupported(participantsMsg.messageHeader))
    {
        peer->SetProtocolVersion(from_header(participantsMsg.messageHeader));
    }
    else
    {
        // Not acknowledged
        NotifyNetworkIncompatibility(participantsMsg.messageHeader, peer->GetInfo().participantName);

        ParticipantAnnouncementReply reply;
        reply.status = ParticipantAnnouncementReply::Status::Failed;
        reply.remoteHeader = to_header(_version);
        peer->SendIbMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
        return;
    }

    _logger->Debug("Received known participants list from IbRegistry protocol {}.{}",
        participantsMsg.messageHeader.versionHigh, participantsMsg.messageHeader.versionLow);

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
        SendParticipantAnnouncement(peer.get());

        // The service ID is incomplete at this stage.
        ServiceDescriptor peerId;
        peerId.SetParticipantName(peerUri.participantName);
        peer->SetServiceDescriptor(peerId);

        const auto result =
            _hashToParticipantName.insert({ib::util::hash::Hash(peerUri.participantName), peerUri.participantName});
        if (result.second == false)
        {
            assert(false);
        }


        AddPeer(std::move(peer));
    };
    // check URI first
    for (auto&& uri : participantsMsg.peerInfos)
    {
        connectPeer(uri);
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
            ib::util::SetThreadName("IB-IOWorker");
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

    // file must not exist before we bind/listen on it
    (void)fs::remove(localEndpoint.path());

    AcceptConnectionsOn(_localAcceptor, localEndpoint);
}

void VAsioConnection::AcceptTcpConnectionsOn(const std::string& hostName, uint16_t port)
{
    // Default to TCP IPv4 catchall
    tcp::endpoint endpoint(tcp::v4(), port);

    auto isIpv4 = [](const auto endpoint) {
        return endpoint.protocol().family() == asio::ip::tcp::v4().family();
    };

    if (! hostName.empty())
    {
        tcp::resolver resolver(_ioContext);
        tcp::resolver::results_type resolverResults;
        try
        {
            resolverResults = resolver.resolve(hostName,std::to_string(static_cast<int>(port)));
            _logger->Debug( "Accepting connections at {}:{} @{}",
                resolverResults->host_name(),
                resolverResults->service_name(),
                (isIpv4(resolverResults->endpoint()) ? "TCPv4" : "TCPv6"));
        }
        catch (const asio::system_error& err)
        {
            _logger->Error("VAsioConnection::AcceptConnectionsOn: Unable to resolve hostname \"{}:{}\": {}", hostName,
                           port, err.what());
            return;
        }

         endpoint = resolverResults->endpoint();
    }

    if (isIpv4(endpoint))
    {
        AcceptConnectionsOn(_tcp4Acceptor, endpoint);
    }
    else
    {
        AcceptConnectionsOn(_tcp6Acceptor, endpoint);
    }
}

template<typename AcceptorT, typename EndpointT>
void VAsioConnection::AcceptConnectionsOn(AcceptorT& acceptor, EndpointT endpoint)
{
    if (acceptor.is_open())
    {
        // we already have an acceptor for the given endpoint type
        std::stringstream endpointName;
        endpointName << endpoint;
        throw std::logic_error{ "VAsioConnection: acceptor already open for endpoint type: " 
            + endpointName.str()};
    }
    try
    {
        acceptor.open(endpoint.protocol());
        SetPlatformOptions(acceptor);
        acceptor.bind(endpoint);
        SetSocketPermissions(endpoint);
        acceptor.listen();
        SetListenOptions(_logger, acceptor);
    }
    catch (const std::exception& e)
    {
        _logger->Error("VAsioConnection failed to listening on {}: {}", endpoint, e.what());
        acceptor = AcceptorT{_ioContext}; // Reset socket
        throw;
    }

    _logger->Debug("VAsioConnection is listening on {}", acceptor.local_endpoint());

    AcceptNextConnection(acceptor);
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

void VAsioConnection::AddPeer(std::shared_ptr<IVAsioPeer> newPeer)
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

    ib::mw::sync::ParticipantStatus msg;
    msg.participantName = info.participantName;
    msg.state = ib::mw::sync::ParticipantState::Error;
    msg.enterReason = "Connection Lost";
    msg.enterTime = std::chrono::system_clock::now();
    msg.refreshTime = std::chrono::system_clock::now();

    auto&& link = GetLinkByName<ib::mw::sync::ParticipantStatus>("default");

    // The VAsioTcpPeer has an incomplete Service ID, fill in the missing
    // link and participant names.
    auto& peerService = dynamic_cast<IIbServiceEndpoint&>(*peer);
    auto peerId = peerService.GetServiceDescriptor();
    peerId.SetParticipantName(peer->GetInfo().participantName);
    peerId.SetNetworkName(link->Name());
    peerService.SetServiceDescriptor(peerId);
    link->DistributeRemoteIbMessage(&peerService, msg);

    _logger->Error("Lost connection to participant {}", peerId);
}

void VAsioConnection::OnSocketData(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto messageKind = buffer.GetMessageKind();
    switch (messageKind)
    {
    case VAsioMsgKind::Invalid:
        _logger->Warn("Received message with VAsioMsgKind::Invalid");
        break;
    case VAsioMsgKind::SubscriptionAnnouncement:
        return ReceiveSubscriptionAnnouncement(from, std::move(buffer));
    case VAsioMsgKind::SubscriptionAcknowledge:
        return ReceiveSubscriptionAcknowledge(from, std::move(buffer));
    case VAsioMsgKind::IbMwMsg:
        return ReceiveRawIbMessage(from, std::move(buffer));
    case VAsioMsgKind::IbSimMsg:
        return ReceiveRawIbMessage(from, std::move(buffer));
    case VAsioMsgKind::IbRegistryMessage:
        return ReceiveRegistryMessage(from, std::move(buffer));
    }
}

void VAsioConnection::ReceiveSubscriptionAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer)
{
    // Note: there may be multiple types that match the SerdesName
    // we try to find a version to match it, for backward compatibility.
    auto getVersionForSerdes = [](const auto& typeName, auto remoteVersion) {
        VersionT subscriptionVersion{0};
        IbMessageTypes supportedMessageTypes{};

        tt::for_each(supportedMessageTypes,
            [&subscriptionVersion, &typeName, remoteVersion](auto&& myType) {
            using MsgT = std::decay_t<decltype(myType)>;
            if (typeName == IbMsgTraits<MsgT>::SerdesName())
            {
                if(IbMsgTraits<MsgT>::Version() <= remoteVersion)
                {
                    subscriptionVersion =  IbMsgTraits<MsgT>::Version();
                }
            }
        });
        return subscriptionVersion;
    };

    auto subscriber = buffer.Deserialize<VAsioMsgSubscriber>();
    bool wasAdded = TryAddRemoteSubscriber(from, subscriber);

    // check our Message version against the remote participant's version
    auto myMessageVersion = getVersionForSerdes(subscriber.msgTypeName, subscriber.version);
    if (myMessageVersion == 0)
    {
        _logger->Warn(
            "Received SubscriptionAnnouncement from {} for message type {}"
            "for an unknown subscriber version {}",
            from->GetInfo().participantName, subscriber.msgTypeName, subscriber.version);
    }
    else
    {
        // Tell our peer what version of the given message type we have
        subscriber.version = myMessageVersion;
    }
    // send acknowledge
    SubscriptionAcknowledge ack;
    ack.subscriber = std::move(subscriber);
    ack.status = wasAdded
        ? SubscriptionAcknowledge::Status::Success
        : SubscriptionAcknowledge::Status::Failed;

    from->SendIbMsg(SerializedMessage{from->GetProtocolVersion(), ack});
}

void VAsioConnection::ReceiveSubscriptionAcknowledge(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto ack = buffer.Deserialize<SubscriptionAcknowledge>();

    if (ack.status != SubscriptionAcknowledge::Status::Success)
    {
        _logger->Error("Failed to subscribe [{}] {} from {}"
            , ack.subscriber.networkName
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

        if (subscriber.msgTypeName != LinkType::MessageSerdesName())
            return;

        auto& ibLink = linkMap[subscriber.networkName];
        if (!ibLink)
        {
            ibLink = std::make_shared<LinkType>(subscriber.networkName, _logger);
        }

        ibLink->AddRemoteReceiver(from, subscriber.receiverIdx);

        wasAdded = true;
    });

    if (wasAdded)
        _logger->Debug("Registered subscription for [{}] {} from {}", subscriber.networkName, subscriber.msgTypeName, from->GetInfo().participantName);
    else
        _logger->Warn("Cannot register subscription for [{}] {} from {}", subscriber.networkName, subscriber.msgTypeName, from->GetInfo().participantName);

    return wasAdded;
}

void VAsioConnection::ReceiveRawIbMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto receiverIdx =  buffer.GetRemoteIndex();//ExtractEndpointId(buffer);
    if (receiverIdx >= _vasioReceivers.size())
    {
        _logger->Warn("Ignoring RawIbMessage for unknown receiverIdx={}", receiverIdx);
        return;
    }

    auto endpoint = buffer.GetEndpointAddress(); //ExtractEndpointAddress(buffer);

    auto* fromService = dynamic_cast<IIbServiceEndpoint*>(from);
    ServiceDescriptor tmpService(fromService->GetServiceDescriptor());
    tmpService.SetServiceId(endpoint.endpoint);

    _vasioReceivers[receiverIdx]->ReceiveRawMsg(from, tmpService, std::move(buffer));
}

void VAsioConnection::RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback)
{
    _participantAnnouncementReceivers.emplace_back(std::move(callback));
}

void VAsioConnection::ReceiveRegistryMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto kind = buffer.GetRegistryKind();
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
        return ReceiveKnownParticpants(from, std::move(buffer));
    }
}

} // namespace mw
} // namespace ib
