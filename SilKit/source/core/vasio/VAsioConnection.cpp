/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "VAsioConnection.hpp"
#include "VAsioProtocolVersion.hpp"

#include "SerializedMessage.hpp"

#include <algorithm>
#include <chrono>
#include <thread>
#include <array>
#include <functional>
#include <cctype>
#include <map>

#include "silkit/services/logging/ILogger.hpp"

#include "VAsioTcpPeer.hpp"
#include "Filesystem.hpp"
#include "SetThreadName.hpp"
#include "Uri.hpp"
#include "Assert.hpp"

using namespace std::chrono_literals;
namespace fs = SilKit::Filesystem;

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
void SetListenOptions(SilKit::Services::Logging::ILogger*,
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
void SetListenOptions(SilKit::Services::Logging::ILogger* logger,
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
void SetListenOptions(SilKit::Services::Logging::ILogger* ,
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
//Debug  print of given peer infos
auto printUris(const SilKit::Core::VAsioPeerInfo& info)
{
    return fmt::format("{}", fmt::join(info.acceptorUris, ", "));
}

auto printUris(const std::vector<std::string>& uris)
{
    return fmt::format("{}", fmt::join(uris, ", "));
}

//!< Note that local ipc (unix domain) sockets have a path limit (108 characters, typically)
// Using the current working directory as part of a domain socket path might result in 
// a runtime exception. We create a unique temporary file path, with a fixed length.
auto makeLocalEndpoint(const std::string& participantName, const SilKit::Core::ParticipantId& id,
                       const std::string& uniqueValue) -> asio::local::stream_protocol::endpoint
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
        + uniqueValue
        + fs::current_path().string()
    );

    std::stringstream path;
    path << fs::temp_directory_path().string()
        << fs::path::preferred_separator
        << bounded_name
        << std::hex << unique_id
        << ".silkit";

    result.path(path.str());
    return result;
}



// end point to string conversions
auto fromAsioEndpoint(const asio::local::stream_protocol::endpoint& ep)
{
    std::stringstream uri;
    const std::string localPrefix{ "local://" };
    uri << localPrefix << ep.path();
    return SilKit::Core::Uri::Parse(uri.str());
}

// end point to string conversions
auto fromAsioEndpoint(const asio::ip::tcp::endpoint& ep)
{
    std::stringstream uri;
    const std::string tcpPrefix{ "tcp://" };
    uri << tcpPrefix << ep; //will be "ipv4:port" or "[ipv6]:port"
    return SilKit::Core::Uri::Parse(uri.str());
}

auto makeLocalPeerInfo(const std::string& name, SilKit::Core::ParticipantId id, const std::string& uniqueDetail)
{
    SilKit::Core::VAsioPeerInfo pi;
    pi.participantName = name;
    pi.participantId = id;
    auto localEp = makeLocalEndpoint(name, id, uniqueDetail);
    pi.acceptorUris.emplace_back(fromAsioEndpoint(localEp).EncodedString());
    return pi;
}

bool connectWithRetry(SilKit::Core::VAsioTcpPeer* peer, const SilKit::Core::VAsioPeerInfo& pi, size_t connectAttempts)
{
    for (auto i = 0u; i < connectAttempts; i++)
    {
        try
        {
            peer->Connect(pi);
            return true;
        }
        catch (const std::exception&)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
        }
    }
    return false;
}

auto selectBestEndpointFromResolverResults(const asio::ip::tcp::resolver::results_type& resolverResults)
    -> asio::ip::tcp::endpoint
{
    // NB: IPv4 should be preferred over IPv6

    std::multimap<int, asio::ip::tcp::endpoint> endpointsByPenalty;

    for (const auto& result : resolverResults)
    {
        const auto resolvedEndpoint = result.endpoint();
        const auto resolvedEndpointAddress = resolvedEndpoint.address();

        if (resolvedEndpointAddress.is_v4())
        {
            endpointsByPenalty.emplace(2000, resolvedEndpoint);
        }
        else if (resolvedEndpointAddress.is_v6())
        {
            endpointsByPenalty.emplace(4000, resolvedEndpoint);
        }
        else
        {
            endpointsByPenalty.emplace(6000, resolvedEndpoint);
        }
    }

    if (endpointsByPenalty.empty())
    {
        throw SilKit::StateError{"Unable to find suitable endpoint."};
    }

    // select the endpoint with the smallest penalty
    return endpointsByPenalty.begin()->second;
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

namespace SilKit {
namespace Core {

namespace tt = Util::tuple_tools;

template <class T> struct Zero { using Type = T; };

using asio::ip::tcp;

VAsioConnection::VAsioConnection(SilKit::Config::ParticipantConfiguration config,
    std::string participantName, ParticipantId participantId,
    Services::Orchestration::ITimeProvider* timeProvider,
    ProtocolVersion version)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
    , _timeProvider{timeProvider}
    , _tcp4Acceptor{_ioContext}
    , _tcp6Acceptor{_ioContext}
    , _localAcceptor{_ioContext}
    , _version{version}
{
    RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { UpdateParticipantStatusOnConnectionLoss(peer); });
    _hashToParticipantName.insert(std::pair<uint64_t, std::string>(SilKit::Util::Hash::Hash(_participantName), _participantName));
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

void VAsioConnection::SetLogger(Services::Logging::ILogger* logger)
{
    _logger = logger;
}

void VAsioConnection::JoinSimulation(std::string connectUri)
{
    SILKIT_ASSERT(_logger);

    if (_config.middleware.enableDomainSockets)
    {
        // We pick a random file name for local domain sockets
        try
        {
            AcceptLocalConnections(connectUri);
        }
        catch (const std::exception& ex)
        {
            _logger->Warn("VasioConnection::JoinSimulation: Cannot accept local IPC connections: {}, pwd={}",
                ex.what(), fs::current_path().string());
        }
    }

    // We let the operating system choose a free TCP port
    // The address will be substituted by the registry, from the actual connection endpoint's address.
    AcceptConnectionsOn(_tcp4Acceptor, tcp::endpoint{asio::ip::tcp::v4(), 0});
    AcceptConnectionsOn(_tcp6Acceptor, tcp::endpoint{asio::ip::tcp::v6(), 0});

    auto registry = VAsioTcpPeer::Create(_ioContext.get_executor(), this, _logger);
    bool ok = false;

    // NB: We attempt to connect multiple times. The registry might be a separate process
    //     which may still be initializing when we are running. For example, this happens when all
    //     participants are started in a shell, and the registry is started in the background.
    const auto connectAttempts = _config.middleware.connectAttempts;

    // Compute a list of Registry URIs and attempt to connect as per config
    std::vector<std::string> attemptedUris{{connectUri}};

    _logger->Debug("Connecting to VAsio registry");

    // First, attempt local connections if available:
    if (_config.middleware.enableDomainSockets)
    {
        auto localPi = makeLocalPeerInfo("SilKitRegistry", 0, connectUri);
        //store local domain acceptor address for printing debug infos later
        attemptedUris.push_back(localPi.acceptorUris.at(0));
        ok = connectWithRetry(registry.get(), localPi, connectAttempts);
    }

    // Fall back to TCP connections:
    if (!ok)
    {
        // setup TCP remote URI
        VAsioPeerInfo pi;
        pi.participantName = "SilKitRegistry";
        pi.participantId = 0;
        pi.acceptorUris.push_back(connectUri);
        ok = connectWithRetry(registry.get(), pi, connectAttempts);
    }
    // Neither local nor tcp is working.
    if (!ok)
    {
        _logger->Error("Failed to connect to VAsio registry (number of attempts: {})",
                       _config.middleware.connectAttempts);
        _logger->Info("   Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: {}.",
            printUris(attemptedUris));
        _logger->Info("   If a registry is unable to open a listening socket it will only be reachable"
                      " via local domain sockets, which depend on the working directory"
                      " and the middleware configuration ('enableDomainSockets').");
        _logger->Info("   Make sure that the hostname can be resolved and is reachable.");
        _logger->Info("   You can configure the SIL Kit Registry hostname and port via the SilKitConfig.");
        _logger->Info("   The SIL Kit Registry executable can be found in your SIL Kit installation folder:");
        _logger->Info("     INSTALL_DIR/bin/sil-kit-registry[.exe]");
        throw SilKitError{"ERROR: Failed to connect to SIL Kit Registry"};
    }

    {
        std::ostringstream ss;
        ss << "Connected to registry at '" << registry->GetRemoteAddress() << "' via '" << registry->GetLocalAddress()
           << "' (" << printUris(registry->GetInfo().acceptorUris) << ')';
        _logger->Info(ss.str());
    }

    registry->StartAsyncRead();

    SendParticipantAnnouncement(registry.get());
    _registry = std::move(registry);

    StartIoWorker();

    auto receivedAllReplies = _receivedAllParticipantReplies.get_future();
    _logger->Debug("VAsio is waiting for known participants list from registry.");

    auto waitOk = receivedAllReplies.wait_for(5s);
    if(waitOk == std::future_status::timeout)
    {
        _logger->Error("Timeout during connection handshake with SIL Kit Registry.");
        throw ProtocolError("Timeout during connection handshake with SIL Kit Registry.");
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
        from->SendSilKitMsg(SerializedMessage{from->GetProtocolVersion(), reply});

        return;
    }

    // after negotiating the Version, we can safely deserialize the remaining message
    // ParticipantAnnouncement announcement;
    buffer.SetProtocolVersion(from->GetProtocolVersion());
    auto announcement = buffer.Deserialize<ParticipantAnnouncement>();

    _logger->Debug("Received participant announcement from {}, protocol version {}.{}", announcement.peerInfo.participantName, announcement.messageHeader.versionHigh,
        announcement.messageHeader.versionLow);

    from->SetInfo(announcement.peerInfo);
    auto& service = dynamic_cast<IServiceEndpoint&>(*from);
    auto serviceDescriptor = service.GetServiceDescriptor();
    serviceDescriptor.SetParticipantName(announcement.peerInfo.participantName);
    service.SetServiceDescriptor(serviceDescriptor);
    {
        std::unique_lock<decltype(_participantAnnouncementReceiversMutex)> lock{_participantAnnouncementReceiversMutex};
        for (auto&& receiver : _participantAnnouncementReceivers)
        {
            receiver(from, announcement);
        }
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
        throw SilKitError{ "VasioConnection: cannot send announcement on TCP: tcp-acceptors for IPv4 and IPv6 are missing" };
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
    peer->SendSilKitMsg(SerializedMessage{announcement});

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
            auto msg = fmt::format("SILKIT Connection Handshake: ParticipantAnnouncementReply contains invalid preambel in header. check endianess on participant {}.", from->GetInfo().participantName);
        }
        const auto msg = fmt::format("SILKIT Connection Handshake: ParticipantAnnouncementReply contains unsupported version."
                        " participant={} participant-version={}",
                        from->GetInfo().participantName, remoteVersion);

        if(from->GetInfo().participantId == RegistryParticipantId)
        {
            // handshake with SIL Kit Registry failed, we need to abort.
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
    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::AddParticipantToLookup(const std::string& participantName)
{
    const auto result = _hashToParticipantName.insert({SilKit::Util::Hash::Hash(participantName), participantName});
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
        throw SilKitError{"VAsioConnection: could not find participant in participant cache"};
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
        peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
        return;
    }

    _logger->Debug("Received known participants list from SilKitRegistry protocol {}.{}",
        participantsMsg.messageHeader.versionHigh, participantsMsg.messageHeader.versionLow);

    auto connectPeer = [this](const auto peerUri) {
        _logger->Debug("Connecting to {} with Id {} on {}",
            peerUri.participantName,
            peerUri.participantId,
            printUris(peerUri));

        auto peer = VAsioTcpPeer::Create(_ioContext.get_executor(), this, _logger);
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
            _hashToParticipantName.insert({SilKit::Util::Hash::Hash(peerUri.participantName), peerUri.participantName});
        if (result.second == false)
        {
            SILKIT_ASSERT(false);
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
            SilKit::Util::SetThreadName("SilKit-IOWorker");
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

void VAsioConnection::AcceptLocalConnections(const std::string& uniqueId)
{
    auto localEndpoint = makeLocalEndpoint(_participantName, _participantId, uniqueId);

    // file must not exist before we bind/listen on it
    (void)fs::remove(localEndpoint.path());

    AcceptConnectionsOn(_localAcceptor, localEndpoint);
}

auto VAsioConnection::AcceptTcpConnectionsOn(const std::string& hostName, uint16_t port)
    -> std::pair<std::string, uint16_t>
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
        }
        catch (const asio::system_error& err)
        {
            _logger->Error("VAsioConnection::AcceptTcpConnectionsOn: Unable to resolve hostname \"{}:{}\": {}", hostName,
                           port, err.what());
            throw SilKit::StateError{"Unable to resolve hostname and service."};
        }

        endpoint = selectBestEndpointFromResolverResults(resolverResults);

        _logger->Debug("Accepting connections at {}:{} @{}",
                       resolverResults->host_name(),
                       resolverResults->service_name(),
                       (isIpv4(endpoint) ? "TCPv4" : "TCPv6"));
    }

    if (isIpv4(endpoint))
    {
        const auto localEndpoint = AcceptConnectionsOn(_tcp4Acceptor, endpoint);
        return std::make_pair(localEndpoint.address().to_string(), localEndpoint.port());
    }
    else
    {
        const auto localEndpoint = AcceptConnectionsOn(_tcp6Acceptor, endpoint);
        return std::make_pair("[" + localEndpoint.address().to_string() + "]", localEndpoint.port());
    }
}

template<typename AcceptorT, typename EndpointT>
auto VAsioConnection::AcceptConnectionsOn(AcceptorT& acceptor, EndpointT endpoint) -> EndpointT
{
    if (acceptor.is_open())
    {
        // we already have an acceptor for the given endpoint type
        std::stringstream endpointName;
        endpointName << endpoint;
        throw LogicError{ "VAsioConnection: acceptor already open for endpoint type: "
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

    return acceptor.local_endpoint();
}

template<typename AcceptorT>
void VAsioConnection::AcceptNextConnection(AcceptorT& acceptor)
{
    std::shared_ptr<VAsioTcpPeer> newConnection;
    try
    {
        newConnection = VAsioTcpPeer::Create(_ioContext.get_executor(), this, _logger);
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

    RemovePeerFromLinks(peer);
    RemovePeerFromConnection(peer);
}

void VAsioConnection::RemovePeerFromLinks(IVAsioPeer* peer)
{
    tt::for_each(_links, [peer](auto&& linkMap) {
        for (auto&& link : linkMap)
        {
            link.second->RemoveRemoteReceiver(peer);
        }
    });
}

void VAsioConnection::RemovePeerFromConnection(IVAsioPeer* peer)
{
    auto it = std::find_if(_peers.begin(), _peers.end(), [peer](auto&& p) {
        auto localPeerInfo = p->GetInfo();
        auto peerToRemove = peer->GetInfo();
        return localPeerInfo.participantId == peerToRemove.participantId;
    });
    if (it != _peers.end())
    {
        _peers.erase(it);
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

    SilKit::Services::Orchestration::ParticipantStatus msg;
    msg.participantName = info.participantName;
    msg.state = SilKit::Services::Orchestration::ParticipantState::Error;
    msg.enterReason = "Connection Lost";
    msg.enterTime = std::chrono::system_clock::now();
    msg.refreshTime = std::chrono::system_clock::now();

    auto&& link = GetLinkByName<SilKit::Services::Orchestration::ParticipantStatus>("default");

    // The VAsioTcpPeer has an incomplete Service ID, fill in the missing
    // link and participant names.
    auto& peerService = dynamic_cast<IServiceEndpoint&>(*peer);
    auto peerId = peerService.GetServiceDescriptor();
    peerId.SetParticipantName(peer->GetInfo().participantName);
    peerId.SetNetworkName(link->Name());
    peerService.SetServiceDescriptor(peerId);
    link->DistributeRemoteSilKitMessage(&peerService, std::move(msg));

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
    case VAsioMsgKind::SilKitMwMsg:
        return ReceiveRawSilKitMessage(from, std::move(buffer));
    case VAsioMsgKind::SilKitSimMsg:
        return ReceiveRawSilKitMessage(from, std::move(buffer));
    case VAsioMsgKind::SilKitRegistryMessage:
        return ReceiveRegistryMessage(from, std::move(buffer));
    }
}

void VAsioConnection::ReceiveSubscriptionAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer)
{
    // Note: there may be multiple types that match the SerdesName
    // we try to find a version to match it, for backward compatibility.
    auto getVersionForSerdes = [](const auto& typeName, auto remoteVersion) {
        VersionT subscriptionVersion{0};
        SilKitMessageTypes supportedMessageTypes{};

        tt::for_each(supportedMessageTypes,
            [&subscriptionVersion, &typeName, remoteVersion](auto&& myType) {
            using MsgT = std::decay_t<decltype(myType)>;
            if (typeName == SilKitMsgTraits<MsgT>::SerdesName())
            {
                if(SilKitMsgTraits<MsgT>::Version() <= remoteVersion)
                {
                    subscriptionVersion =  SilKitMsgTraits<MsgT>::Version();
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

    from->SendSilKitMsg(SerializedMessage{from->GetProtocolVersion(), ack});
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

    // We remove the pending subscription in any case as there will not follow a new, successful acknowledge from that peer
    RemovePendingSubscription({from, ack.subscriber});
}

void VAsioConnection::RemovePendingSubscription(const PendingAcksIdentifier& ackId)
{
    auto iterPendingSync =
        std::find(_pendingSubscriptionAcknowledges.begin(), _pendingSubscriptionAcknowledges.end(), ackId);
    if (iterPendingSync != _pendingSubscriptionAcknowledges.end())
    {
        _pendingSubscriptionAcknowledges.erase(iterPendingSync);
        if (_pendingSubscriptionAcknowledges.empty())
        {
            SyncSubscriptionsCompleted();
        }
    }

    auto iterPendingASync =
        std::find(_pendingAsyncSubscriptionAcknowledges.begin(), _pendingAsyncSubscriptionAcknowledges.end(), ackId);
    if (iterPendingASync != _pendingAsyncSubscriptionAcknowledges.end())
    {
        _pendingAsyncSubscriptionAcknowledges.erase(iterPendingASync);
        if (_pendingAsyncSubscriptionAcknowledges.empty())
        {
            AsyncSubscriptionsCompleted();
        }
    }
}

bool VAsioConnection::TryAddRemoteSubscriber(IVAsioPeer* from, const VAsioMsgSubscriber& subscriber)
{
    bool wasAdded = false;

    tt::for_each(_links, [&](auto&& linkMap) {
        using LinkType = typename std::decay_t<decltype(linkMap)>::mapped_type::element_type;

        if (subscriber.msgTypeName != LinkType::MessageSerdesName())
            return;

        auto& link = linkMap[subscriber.networkName];
        if (!link)
        {
            link = std::make_shared<LinkType>(subscriber.networkName, _logger, _timeProvider);
        }

        link->AddRemoteReceiver(from, subscriber.receiverIdx);

        wasAdded = true;
    });

    if (wasAdded)
        _logger->Debug("Registered subscription for [{}] {} from {}", subscriber.networkName, subscriber.msgTypeName, from->GetInfo().participantName);
    else
        _logger->Warn("Cannot register subscription for [{}] {} from {}", subscriber.networkName, subscriber.msgTypeName, from->GetInfo().participantName);

    return wasAdded;
}

void VAsioConnection::ReceiveRawSilKitMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto receiverIdx =  buffer.GetRemoteIndex();//ExtractEndpointId(buffer);
    if (receiverIdx >= _vasioReceivers.size())
    {
        _logger->Warn("Ignoring RawSilKitMessage for unknown receiverIdx={}", receiverIdx);
        return;
    }

    auto endpoint = buffer.GetEndpointAddress(); //ExtractEndpointAddress(buffer);

    auto* fromService = dynamic_cast<IServiceEndpoint*>(from);
    ServiceDescriptor tmpService(fromService->GetServiceDescriptor());
    tmpService.SetServiceId(endpoint.endpoint);

    _vasioReceivers[receiverIdx]->ReceiveRawMsg(from, tmpService, std::move(buffer));
}

void VAsioConnection::RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback)
{
    std::unique_lock<decltype(_participantAnnouncementReceiversMutex)> lock{_participantAnnouncementReceiversMutex};
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

void VAsioConnection::SetAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    if (_hasPendingAsyncSubscriptions)
    {
        _asyncSubscriptionsCompletionHandler = std::move(handler);
    }
    else
    {
        handler(); 
    }
}

void VAsioConnection::SyncSubscriptionsCompleted()
{
    _receivedAllSubscriptionAcknowledges.set_value();
}

void VAsioConnection::AsyncSubscriptionsCompleted()
{
    if (_asyncSubscriptionsCompletionHandler)
    {
        _asyncSubscriptionsCompletionHandler();
        _asyncSubscriptionsCompletionHandler = nullptr;
    }
    _hasPendingAsyncSubscriptions = false;
}

} // namespace Core
} // namespace SilKit
