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

#include "ILogger.hpp"
#include "VAsioTcpPeer.hpp"
#include "VAsioProxyPeer.hpp"
#include "Filesystem.hpp"
#include "SetThreadName.hpp"
#include "Uri.hpp"
#include "Assert.hpp"
#include "TransformAcceptorUris.hpp"

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
        SilKit::Services::Logging::Warn(logger, "SetListenOptions: Setting Loopback FastPath failed: WSA IOCtl last error: {}", lastError);
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

//! Note that local ipc (unix domain) sockets have a path limit (108 characters, typically)
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
        bool success = false;
        std::stringstream attemptedUris;
        peer->Connect(pi, attemptedUris, success);
        if (success)
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
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

auto GetCurrentCapabilities(const SilKit::Config::ParticipantConfiguration& participantConfiguration) -> std::string
{
    SilKit::Core::VAsioCapabilities capabilities;

    if (participantConfiguration.middleware.registryAsFallbackProxy)
    {
        capabilities.AddCapability(SilKit::Core::Capabilities::ProxyMessage);
    }

    capabilities.AddCapability(SilKit::Core::Capabilities::AutonomousSynchronous);
    capabilities.AddCapability(SilKit::Core::Capabilities::RequestParticipantConnection);

    return capabilities.ToCapabilitiesString();
}

bool PeerHasCapability(const SilKit::Core::VAsioPeerInfo& peerInfo, const SilKit::Core::CapabilityLiteral& capability)
{
    SilKit::Core::VAsioCapabilities capabilities{peerInfo.capabilities};
    return capabilities.HasCapability(capability);
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

MAKE_FORMATTER(asio::generic::stream_protocol::socket);

namespace SilKit {
namespace Core {

namespace tt = Util::tuple_tools;

template <class T> struct Zero { using Type = T; };

using asio::ip::tcp;

VAsioConnection::VAsioConnection(
    IParticipantInternal* participant,
    SilKit::Config::ParticipantConfiguration config,
    std::string participantName, ParticipantId participantId,
    Services::Orchestration::ITimeProvider* timeProvider,
    ProtocolVersion version)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
    , _timeProvider{timeProvider}
    , _version{version}
    , _participant{participant}
{
    _hashToParticipantName.insert(std::pair<uint64_t, std::string>(SilKit::Util::Hash::Hash(_participantName), _participantName));
}

VAsioConnection::~VAsioConnection()
{
    _isShuttingDown = true;

    std::unique_lock<std::mutex> lock{_peersLock};
    decltype(_peers) peers;
    peers.swap(_peers);
    for (auto peer : peers)
    {
        peer->DrainAllBuffers();
    }
    lock.unlock();

    if (_ioWorker.joinable())
    {
        _ioContext.stop();
        _ioWorker.join();
    }

    // clean up local ipc sockets
    for (const auto & acceptor : _localAcceptors)
    {
        if (acceptor.is_open())
        {
            (void)fs::remove(acceptor.local_endpoint().path());
        }
    }
}

void VAsioConnection::SetLogger(Services::Logging::ILogger* logger)
{
    _logger = logger;
}

auto VAsioConnection::PrepareAcceptorEndpointUris(const std::string & connectUri) -> std::vector<std::string>
{
    auto acceptorEndpointUris = _config.middleware.acceptorUris;

    if (acceptorEndpointUris.empty())
    {
        // We let the operating system choose a free TCP port.
        // The address will be substituted by the registry, from the actual connection endpoint's address.
        acceptorEndpointUris.emplace_back("tcp://0.0.0.0:0");
        acceptorEndpointUris.emplace_back("tcp://[::]:0");

        // Create the default local-domain socket path.
        auto localEndpoint = makeLocalEndpoint(_participantName, _participantId, connectUri);
        acceptorEndpointUris.emplace_back("local://" + localEndpoint.path());
    }

    return acceptorEndpointUris;
}

void VAsioConnection::OpenTcpAcceptors(const std::vector<std::string> & acceptorEndpointUris)
{
    for (const auto& uriString : acceptorEndpointUris)
    {
        const auto uri = Uri::Parse(uriString);

        if (uri.Type() == Uri::UriType::Tcp && uri.Scheme() == "tcp")
        {
            SilKit::Services::Logging::Debug(_logger, "Found TCP acceptor endpoint URI {} with host {} and port {}",
                                             uriString, uri.Host(), uri.Port());

            for (const auto& result : ResolveHostAndPort(_ioContext.get_executor(), _logger, uri.Host(), uri.Port()))
            {
                const auto endpoint = result.endpoint();
                const auto address = endpoint.address();

                if (address.is_v4() || address.is_v6())
                {
                    Services::Logging::Debug(_logger, "Accepting {} connections on {}:{}",
                                             (address.is_v4() ? "TCPv4" : "TCPv6"), uri.Host(), uri.Port());

                    _tcpAcceptors.emplace_back(_ioContext);
                    auto& acceptor = _tcpAcceptors.back();

                    try
                    {
                        AcceptConnectionsOn(acceptor, endpoint);
                    }
                    catch (const std::exception& exception)
                    {
                        Services::Logging::Error(_logger, "Unable to accept {} connections on {}:{}: {}",
                                                 (address.is_v4() ? "TCPv4" : "TCPv6"), uri.Host(), uri.Port(),
                                                 exception.what());
                        _tcpAcceptors.pop_back();
                    }
                }
                else
                {
                    Services::Logging::Debug(_logger, "Not accepting connection on {}", endpoint);
                }
            }
        }
        else if (uri.Type() == Uri::UriType::Local && uri.Scheme() == "local")
        {
            // do nothing, handled elsewhere
        }
        else
        {
            SilKit::Services::Logging::Warn(
                _logger, "OpenTcpAcceptors: Unused acceptor endpoint URI: {}", uriString);
        }
    }
}

void VAsioConnection::OpenLocalAcceptors(const std::vector<std::string> & acceptorEndpointUris)
{
    for (const auto& uriString : acceptorEndpointUris)
    {
        const auto uri = Uri::Parse(uriString);

        if (uri.Type() == Uri::UriType::Local && uri.Scheme() == "local")
        {
            SilKit::Services::Logging::Debug(_logger, "Found local domain acceptor endpoint URI {} with path {}",
                                             uriString, uri.Path());

            asio::local::stream_protocol::endpoint endpoint{uri.Path()};

            // file must not exist before we bind/listen on it
            (void)fs::remove(endpoint.path());

            _localAcceptors.emplace_back(_ioContext);
            auto& acceptor = _localAcceptors.back();

            try
            {
                AcceptConnectionsOn(acceptor, endpoint);
            }
            catch (const std::exception& exception)
            {
                Services::Logging::Error(_logger, "Unable to accept local domain connections on {}:{}: {}", uri.Host(),
                                         uri.Port(), exception.what());
                _localAcceptors.pop_back();
            }
        }
        else if (uri.Type() == Uri::UriType::Tcp && uri.Scheme() == "tcp")
        {
            // do nothing, handled elsewhere
        }
        else
        {
            SilKit::Services::Logging::Warn(
                _logger, "OpenLocalAcceptors: Unused acceptor endpoint URI: {}", uriString);
        }
    }
}

void VAsioConnection::JoinSimulation(std::string connectUri)
{
    SILKIT_ASSERT(_logger);

    const auto acceptorEndpointUris = PrepareAcceptorEndpointUris(connectUri);

    if (_config.middleware.enableDomainSockets)
    {
        OpenLocalAcceptors(acceptorEndpointUris);
    }

    // Accept TCP connections on endpoints given by matching URIs
    OpenTcpAcceptors(acceptorEndpointUris);

    if (_localAcceptors.empty() && _tcpAcceptors.empty())
    {
        SilKit::Services::Logging::Error(_logger, "JoinSimulation: no acceptors available");
        throw SilKitError{"JoinSimulation: no acceptors available"};
    }

    auto registry = VAsioTcpPeer::Create(_ioContext.get_executor(), this, _logger);
    bool ok = false;

    // NB: We attempt to connect multiple times. The registry might be a separate process
    //     which may still be initializing when we are running. For example, this happens when all
    //     participants are started in a shell, and the registry is started in the background.
    const auto connectAttempts = _config.middleware.connectAttempts;

    // Compute a list of Registry URIs and attempt to connect as per config
    std::vector<std::string> attemptedUris{{connectUri}};

    _logger->Debug("Connecting to SIL Kit Registry");

    // First, attempt local connections if available:
    if (_config.middleware.enableDomainSockets)
    {
        auto localPi = makeLocalPeerInfo("SilKitRegistry", RegistryParticipantId, connectUri);
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
        pi.participantId = RegistryParticipantId;
        pi.acceptorUris.push_back(connectUri);
        ok = connectWithRetry(registry.get(), pi, connectAttempts);
    }
    // Neither local nor tcp is working.
    if (!ok)
    {
        Services::Logging::Error(_logger, "Failed to connect to SIL Kit Registry (number of attempts: {})",
                       _config.middleware.connectAttempts);
        Services::Logging::Info(_logger, "   Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: {}.",
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
    _logger->Debug("SIL Kit is waiting for known participants list from registry.");

    auto waitOk = receivedAllReplies.wait_for(5s);
    if(waitOk == std::future_status::timeout)
    {
        if (_hasReceivedKnownParticipants)
        {
            // known participants was received -> probably connection issues with a participant
            bool firstPeer = true;
            std::ostringstream listOfPendingParticipants;
            listOfPendingParticipants << "Timeout during connection setup. The participant was able to connect to the registry, "
                                         "but not to all participants. There might be network issues. Check network settings and "
                                         "firewall configuration. Was not able to connect to the following participant(s): ";

            for (auto& peer : _pendingParticipantReplies)
            {
                auto& peerInfo = peer->GetInfo();
                if (!firstPeer)
                {
                    listOfPendingParticipants << ", ";
                }
                listOfPendingParticipants << peerInfo.participantName << "(";

                bool firstUri = true;
                for (auto& uri : peerInfo.acceptorUris)
                {
                    if (!firstUri)
                    {
                        listOfPendingParticipants << ", ";
                    }
                    listOfPendingParticipants << uri;
                    firstUri = false;
                }

                listOfPendingParticipants << ")";
                firstPeer = false;
            }

            auto message = listOfPendingParticipants.str();
            _logger->Error(message);
            throw ProtocolError(std::move(message));
        }
        else
        {
            // no known participants were received -> probably connection issue with registry
            auto message =
                fmt::format("Timeout during connection handshake with SIL Kit Registry. This might indicate "
                            "that a participant with the same name ('{}') has already connected to the registry.",
                            _participantName);
            _logger->Error(message);
            throw ProtocolError(std::move(message));
        }
    }

    // check if an exception was set:
    receivedAllReplies.get();

    _logger->Trace("SIL Kit received announcement replies from all participants.");
}

void VAsioConnection::NotifyNetworkIncompatibility(const RegistryMsgHeader& other,
                                                   const std::string& otherParticipantName)
{
    const auto errorMsg = fmt::format("Network incompatibility between this version range ({})"
        " and connecting participant '{}' ({})",
        MapVersionToRelease(MakeRegistryMsgHeader(_version)),
        otherParticipantName,
        MapVersionToRelease(other)
    );
    _logger->Critical(errorMsg);
    std::cerr << "ERROR: " << errorMsg << std::endl;
}

void VAsioConnection::SendParticipantAnnouncement(IVAsioPeer* peer)
{
    // Legacy Info for interop
    // URI encoded infos
    VAsioPeerInfo info{_participantName, _participantId, {}, GetCurrentCapabilities(_config)};

    size_t openAcceptorCount = 0;

    // Ensure that the local acceptors are the first entries in the acceptorUris
    for (const auto& acceptor : _localAcceptors)
    {
        if (acceptor.is_open())
        {
            ++openAcceptorCount;

            const auto epUri = fromAsioEndpoint(acceptor.local_endpoint());
            info.acceptorUris.push_back(epUri.EncodedString());
            Services::Logging::Trace(
                _logger, "SendParticipantAnnouncement: Peer '{}': Local-Domain Acceptor Uri: {}",
                peer->GetInfo().participantName, epUri.EncodedString());
        }
    }

    for (const auto& acceptor : _tcpAcceptors)
    {
        if (acceptor.is_open())
        {
            ++openAcceptorCount;

            const auto epUri = fromAsioEndpoint(acceptor.local_endpoint());
            info.acceptorUris.emplace_back(epUri.EncodedString());
            Services::Logging::Trace(_logger,
                                     "SendParticipantAnnouncement: Peer '{}': TCP Acceptor Uri: {}",
                                     peer->GetInfo().participantName, epUri.EncodedString());
        }
    }

    if (openAcceptorCount == 0)
    {
        const auto message = "SendParticipantAnnouncement: Cannot send announcement: All acceptors "
                             "(both Local-Domain and TCP) are missing";
        SilKit::Services::Logging::Error(_logger, message);
        throw SilKitError{message};
    }

    ParticipantAnnouncement announcement;
    // We override the default protocol version here for integration testing
    announcement.messageHeader = MakeRegistryMsgHeader(_version);
    announcement.peerInfo = std::move(info);

    Services::Logging::Debug(_logger, "Sending participant announcement to {}", peer->GetInfo().participantName);
    peer->SendSilKitMsg(SerializedMessage{announcement});
}

void VAsioConnection::ReceiveParticipantAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer)
{
    const auto remoteHeader = buffer.GetRegistryMessageHeader();

    // check if we support the remote peer's protocol version or signal a handshake failure
    if (ProtocolVersionSupported(remoteHeader))
    {
        from->SetProtocolVersion(ExtractProtocolVersion(remoteHeader));
    }
    else
    {
        // it is not safe to decode the ParticipantAnnouncement
        NotifyNetworkIncompatibility(remoteHeader, from->GetRemoteAddress());

        SendFailedParticipantAnnouncementReply(
            from,
            // tell the remote peer the protocol version we intend to speak
            _version,
            // inform the remote peer about the unsupported protocol version
            fmt::format("protocol version {}.{} is not supported", remoteHeader.versionHigh, remoteHeader.versionLow));

        return;
    }

    // after negotiating the Version, we can safely deserialize the remaining message
    buffer.SetProtocolVersion(from->GetProtocolVersion());
    auto announcement = buffer.Deserialize<ParticipantAnnouncement>();

    Services::Logging::Debug(_logger, "Received participant announcement from {}, protocol version {}.{}",
                             announcement.peerInfo.participantName, announcement.messageHeader.versionHigh,
                             announcement.messageHeader.versionLow);

    from->SetInfo(announcement.peerInfo);
    auto& service = dynamic_cast<IServiceEndpoint&>(*from);
    auto serviceDescriptor = service.GetServiceDescriptor();
    serviceDescriptor.SetParticipantNameAndComputeId(announcement.peerInfo.participantName);
    service.SetServiceDescriptor(serviceDescriptor);

    // If one of the handlers for ParticipantAnnouncements throws an exception, report failure to the remote peer
    try
    {
        std::unique_lock<decltype(_participantAnnouncementReceiversMutex)> lock{_participantAnnouncementReceiversMutex};
        for (auto&& receiver : _participantAnnouncementReceivers)
        {
            receiver(from, announcement);
        }
    }
    catch (const SilKitError& error)
    {
        SendFailedParticipantAnnouncementReply(
            from,
            // tell the remote peer what protocol version we accepted for communication with them
            from->GetProtocolVersion(),
            // use the exception message as the diagnostic
            error.what());

        return;
    }

    AssociateParticipantNameAndPeer(announcement.peerInfo.participantName, from);
    SendParticipantAnnouncementReply(from);

    RemovePeerFromPendingLists(from);
}

void VAsioConnection::SendParticipantAnnouncementReply(IVAsioPeer* peer)
{
    ParticipantAnnouncementReply reply;
    // tell the remote peer what *our* protocol version is that we can accept for this peer
    reply.remoteHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());
    reply.status = ParticipantAnnouncementReply::Status::Success;
    // fill in the service descriptors we want to subscribe to
    std::transform(_vasioReceivers.begin(), _vasioReceivers.end(), std::back_inserter(reply.subscribers),
                   [](const auto& subscriber) {
                       return subscriber->GetDescriptor();
                   });

    Services::Logging::Debug(_logger, "Sending ParticipantAnnouncementReply to '{}' with protocol version {}",
                             peer->GetInfo().participantName, ExtractProtocolVersion(reply.remoteHeader));

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::SendFailedParticipantAnnouncementReply(IVAsioPeer* peer, ProtocolVersion version,
                                                             std::string diagnostic)
{
    ParticipantAnnouncementReply reply;
    reply.remoteHeader = MakeRegistryMsgHeader(version);
    reply.status = ParticipantAnnouncementReply::Status::Failed;
    reply.diagnostic = std::move(diagnostic);

    Services::Logging::Debug(_logger, "Sending failed ParticipantAnnouncementReply to '{}' with protocol version {}",
                             peer->GetInfo().participantName, ExtractProtocolVersion(reply.remoteHeader));

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::ReceiveParticipantAnnouncementReply(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto reply = buffer.Deserialize<ParticipantAnnouncementReply>();
    const auto& remoteVersion = ExtractProtocolVersion(reply.remoteHeader);

    if (reply.status == ParticipantAnnouncementReply::Status::Failed)
    {
        const auto message =
            fmt::format("SIL Kit Connection Handshake: Received failed ParticipantAnnouncementReply from '{}' with "
                        "protocol version {} and diagnostic message: {}",
                        from->GetInfo().participantName,
                        // extract the version delivered in the reply (
                        ExtractProtocolVersion(reply.remoteHeader),
                        // provide a non-empty default string for the diagnostic message
                        reply.diagnostic.empty() ? "(no diagnostic message was delivered)" : reply.diagnostic.c_str());

        _logger->Warn(message);

        // tear down the participant if we are talking to the registry
        if (from->GetInfo().participantId == RegistryParticipantId)
        {
            // handshake with SIL Kit Registry failed, we need to abort.
            auto error = ProtocolError(message);
            _receivedAllParticipantReplies.set_exception(std::make_exception_ptr(error)); //propagate to main thread
            throw error; // for I/O thread
        }

        // fail the handshake without tearing down this participant if we are not talking to the registry
        return;
    }

    from->SetProtocolVersion(remoteVersion);
    for (auto& subscriber : reply.subscribers)
    {
        TryAddRemoteSubscriber(from, subscriber);
    }

    Services::Logging::Debug(_logger, "Received participant announcement reply from {} protocol version {}",
                             from->GetInfo().participantName, remoteVersion);
    
    RemovePeerFromPendingLists(from);
}

bool VAsioConnection::TryRequestRemoteConnection(std::shared_ptr<VAsioTcpPeer>& directPeer,
    const VAsioPeerInfo& peerInfo,
    const std::string& message)
{
    if (!PeerHasCapability(peerInfo, Capabilities::RequestParticipantConnection))
    {
        return false;
    }

    SilKit::Services::Logging::Info(
        _logger,
        "VAsioConnection: Failed to connect directly to {}, trying to request a remote connection from the participant via the registry: {}",
        peerInfo.participantName, message);

    // Remove the "direct-connection" peer from the list of peers we expect an answer from
    auto it = std::find(_pendingParticipantReplies.begin(), _pendingParticipantReplies.end(), directPeer);
    _pendingParticipantReplies.erase(it);
    // Destroy the peer object
    directPeer.reset();

    {
        std::unique_lock<decltype(_pendingRemoteMx)> lock(_pendingRemoteMx);
        PendingRemoteConnection remoteConn{_ioContext, peerInfo};
        remoteConn.timer->expires_after(_remoteConnectionTimeout);
        remoteConn.timer->async_wait([this](auto&& ec) {
            return this->HandleExpiredConnection(ec);
        });
        _pendingRemoteConnections[peerInfo.participantName] = std::move(remoteConn);
    }

    //send remote connection request to registry, which it will relay 

    RemoteParticipantConnectRequest request{};
    request.connectTargetPeer = peerInfo;
    request.peerUnableToConnect = {}; // will be replaced by the registry
    request.peerUnableToConnect.participantName = _participantName;
    request.peerUnableToConnect.participantId = _participantId;

    _registry->SendSilKitMsg(SerializedMessage{_registry->GetProtocolVersion(), request});

    return true;
}

bool VAsioConnection::TryCreatingProxy(std::shared_ptr<VAsioTcpPeer>& directPeer,
                                       std::shared_ptr<IVAsioConnectionPeer>& peer,
                                       const VAsioPeerInfo& peerInfo,
                                       const std::string& message)
{
    if (!_config.middleware.registryAsFallbackProxy)
    {
        SilKit::Services::Logging::Warn(_logger,
                                        "VAsioConnection: Cannot use ProxyMessage to communicate with {}, "
                                        "because it is disabled in the configuration",
                                        peerInfo.participantName);
        return false;
    }

    SilKit::Services::Logging::Warn(
        _logger,
        "VAsioConnection: Failed to connect directly to {}, trying to proxy messages through the registry: {}",
        peerInfo.participantName, message);

    // NB: Cannot check the capabilities of the registry, since we do not receive the PeerInfo from the
    //       registry over the network, but build it ourselves in VAsioConnection::JoinSimulation.
    //       This is not be a huge issue, since we can just 'throw the messages at the registry' and will
    //       fail with the participant-connection-timeout if it is not capable of routing it to the other
    //       participant.


    // To use the ProxyMessage, the peer we're trying to connect to must support it
    if (!PeerHasCapability(peerInfo, Capabilities::ProxyMessage))
    {
        SilKit::Services::Logging::Warn(_logger,
                                        "VAsioConnection: Cannot use ProxyMessage to communicate with {}, "
                                        "because {} does not support it",
                                        peerInfo.participantName, peerInfo.participantName);
        return false;
    }

    // Remove the "direct-connection" peer from the list of peers we expect an answer from
    auto it = std::find(_pendingParticipantReplies.begin(), _pendingParticipantReplies.end(), directPeer);
    _pendingParticipantReplies.erase(it);
    // Destroy the peer object
    directPeer = nullptr;

    // Create the "proxy-peer" object
    peer = std::make_shared<VAsioProxyPeer>(this, peerInfo, _registry.get(), _logger);
    // Remember that we expect a reply from this peer
    _pendingParticipantReplies.push_back(peer);
    return true;
}

void VAsioConnection::ReceiveKnownParticpants(IVAsioPeer* peer, SerializedMessage&& buffer)
{
    auto participantsMsg = buffer.Deserialize<KnownParticipants>();

    // After receiving a ParticipantAnnouncement the Registry will send a KnownParticipants message
    // check if we support its version here
    if (ProtocolVersionSupported(participantsMsg.messageHeader))
    {
        peer->SetProtocolVersion(ExtractProtocolVersion(participantsMsg.messageHeader));
    }
    else
    {
        // Not acknowledged
        NotifyNetworkIncompatibility(participantsMsg.messageHeader, peer->GetInfo().participantName);

        ParticipantAnnouncementReply reply;
        reply.status = ParticipantAnnouncementReply::Status::Failed;
        reply.remoteHeader = MakeRegistryMsgHeader(_version);
        peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
        return;
    }

    _hasReceivedKnownParticipants = true;

    Services::Logging::Debug(_logger, "Received known participants list from SilKitRegistry protocol {}.{}",
                             participantsMsg.messageHeader.versionHigh, participantsMsg.messageHeader.versionLow);

    // check URI first
    for (auto&& peerInfo : participantsMsg.peerInfos)
    {
        if (peerInfo.participantName == _participantName)
        {
            continue;
        }
        ConnectPeer(peerInfo);
    }

    if (_pendingParticipantReplies.empty())
    {
        _receivedAllParticipantReplies.set_value();
    }
    Services::Logging::Trace(_logger, "SIL Kit is waiting for {} ParticipantAnnouncementReplies",
                             _pendingParticipantReplies.size());

}



void VAsioConnection::ReceiveRemoteParticipantConnectRequest(SerializedMessage&& buffer)
{
    auto remoteConnectRequest = buffer.Deserialize<RemoteParticipantConnectRequest>();
    if (remoteConnectRequest.connectTargetPeer.participantName == _participantName)
    {
        {
            std::unique_lock<decltype(_peersLock)> lock2(_peersLock);
            for (auto&& connectedPeer : _peers)

            {
                if (connectedPeer->GetInfo().participantName
                    == remoteConnectRequest.peerUnableToConnect.participantName)
                {
                    // we already have a connection to the peer, skip it
                    return;
                }
            }
        }
        ConnectPeer(remoteConnectRequest.peerUnableToConnect, true);
        return;
    }


    // The registry delegates the request to the remote participant
    if (_participantId == RegistryParticipantId)
    {
        // find the peer which was unable to connect, and get its (resolved) peerInfo
        const auto unhappyPeerIt =
            _participantNameToPeer.find(remoteConnectRequest.peerUnableToConnect.participantName);
        if (unhappyPeerIt == _participantNameToPeer.end())
        {
            Services::Logging::Error(_logger,
                                     "SilKit::VAsioConnection: remote connection request from unknown participant '{}'",
                                     remoteConnectRequest.peerUnableToConnect.participantName);
            return;
        }
        const auto peerIt = _participantNameToPeer.find(remoteConnectRequest.connectTargetPeer.participantName);
        if (peerIt == _participantNameToPeer.end())
        {
            Services::Logging::Error(_logger, "SilKit::VAsioConnection: remote connection request to unknown participant '{}'",
                                     remoteConnectRequest.connectTargetPeer.participantName);
            return;
        }
        remoteConnectRequest.peerUnableToConnect = unhappyPeerIt->second->GetInfo();
        remoteConnectRequest.peerUnableToConnect.acceptorUris =
            TransformAcceptorUris(_logger, unhappyPeerIt->second, peerIt->second);
        remoteConnectRequest.peerUnableToConnect.acceptorUris.push_back(unhappyPeerIt->second->GetLocalAddress());

        peerIt->second->SendSilKitMsg(SerializedMessage{std::move(remoteConnectRequest)});
    }
}

void VAsioConnection::ConnectPeer(const VAsioPeerInfo& peerInfo, bool connectDirectly)
{
    Services::Logging::Debug(_logger, "Connecting to {} with Id {} on {}", peerInfo.participantName,
                             peerInfo.participantId, printUris(peerInfo));

    // Create the "direct-connection" peer
    auto directPeer = VAsioTcpPeer::Create(_ioContext.get_executor(), this, _logger);

    // Remember that we expect a reply from this peer
    _pendingParticipantReplies.push_back(directPeer);

    std::shared_ptr<IVAsioConnectionPeer> peer;

    // Try to connect to the peer only _after_ remembering that we need to connect, otherwise suitable error will
    // be raised.
    bool success = false;
    std::stringstream attemptedUris;
    directPeer->Connect(peerInfo, attemptedUris, success);
    if (!success && !connectDirectly)
    {
        auto errorMsg = fmt::format("Failed to connect to host URIs: \"{}\"", attemptedUris.str());
        if (!TryCreatingProxy(directPeer, peer, peerInfo, errorMsg))
        {
            TryRequestRemoteConnection(directPeer, peerInfo, errorMsg);
            return;
        }
    }
    else
    {
        peer = std::move(directPeer);
    }

    // We connected to the other peer. tell him who we are.
    SendParticipantAnnouncement(peer.get());

    // The service ID is incomplete at this stage.
    ServiceDescriptor peerId;
    peerId.SetParticipantNameAndComputeId(peerInfo.participantName);
    peer->SetServiceDescriptor(peerId);

    const auto result =
        _hashToParticipantName.insert({SilKit::Util::Hash::Hash(peerInfo.participantName), peerInfo.participantName});
    if (result.second == false)
    {
        throw SilKit::AssertionError{"VAsioConnection: could not add participant name to hash map"};
    }

    AssociateParticipantNameAndPeer(peer->GetInfo().participantName, peer.get());
    AddPeer(std::move(peer));
}

void VAsioConnection::AssociateParticipantNameAndPeer(const std::string& participantName, IVAsioPeer* peer)
{
    _participantNameToPeer.insert({participantName, peer});
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
            Services::Logging::Error(_logger, "SilKit-IOWorker: Something went wrong: {}", error.what());
            return -1;
        }
    }};
}

void VAsioConnection::AcceptLocalConnections(const std::string& uniqueId)
{
    auto localEndpoint = makeLocalEndpoint(_participantName, _participantId, uniqueId);

    // file must not exist before we bind/listen on it
    (void)fs::remove(localEndpoint.path());

    _localAcceptors.emplace_back(_ioContext);
    auto &acceptor = _localAcceptors.back();

    AcceptConnectionsOn(acceptor, localEndpoint);
}

auto VAsioConnection::AcceptTcpConnectionsOn(const std::string& hostName, uint16_t port)
    -> std::pair<std::string, uint16_t>
{
    // Default to TCP IPv4 catchallIp
    tcp::endpoint endpoint(tcp::v4(), port);

    auto isIpv4 = [](const auto endpoint) {
        return endpoint.protocol().family() == asio::ip::tcp::v4().family();
    };

    if (! hostName.empty())
    {
        tcp::resolver::results_type resolverResults;
        resolverResults = ResolveHostAndPort(_ioContext.get_executor(), _logger, hostName, port);
        if (resolverResults.empty())
        {
            Services::Logging::Error(_logger, "AcceptTcpConnectionsOn: Unable to resolve hostname"
                "\"{}:{}\"", hostName, port);
            throw SilKit::StateError{"Unable to resolve hostname and service."};
        }

        endpoint = selectBestEndpointFromResolverResults(resolverResults);

        Services::Logging::Debug(_logger, "Accepting connections at {}:{} @{}",
                       resolverResults->host_name(),
                       resolverResults->service_name(),
                       (isIpv4(endpoint) ? "TCPv4" : "TCPv6"));
    }

    _tcpAcceptors.emplace_back(_ioContext);
    auto &acceptor = _tcpAcceptors.back();

    const auto localEndpoint = AcceptConnectionsOn(acceptor, endpoint);
    return std::make_pair(localEndpoint.address().to_string(), localEndpoint.port());
}

template<typename AcceptorT, typename EndpointT>
auto VAsioConnection::AcceptConnectionsOn(AcceptorT& acceptor, EndpointT endpoint) -> EndpointT
{
    if (acceptor.is_open())
    {
        // we already have an acceptor for the given endpoint type
        std::stringstream endpointName;
        endpointName << endpoint;
        throw LogicError{ "AcceptConnectionsOn: acceptor already open for endpoint type: "
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
        Services::Logging::Error(_logger, "SIL Kit failed to listening on {}: {}", endpoint, e.what());
        acceptor = AcceptorT{_ioContext}; // Reset socket
        throw;
    }

    Services::Logging::Debug(_logger, "SIL Kit is listening on {}", acceptor.local_endpoint());

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
        Services::Logging::Error(_logger, "SIL Kit cannot create listener socket: {}", e.what());
        throw;
    }

    acceptor.async_accept(newConnection->Socket(),
        [this, newConnection, &acceptor](const asio::error_code& error) mutable
        {
            if (!error)
            {
                Services::Logging::Debug(_logger, "New connection from {}", newConnection->Socket());
                AddPeer(std::move(newConnection));
            }
            AcceptNextConnection(acceptor);
        }
    );
}

void VAsioConnection::AddPeer(std::shared_ptr<IVAsioPeer> newPeer)
{
    newPeer->StartAsyncRead();

    std::unique_lock<std::mutex> lock{_peersLock};

    auto * const proxyPeer = dynamic_cast<VAsioProxyPeer *>(newPeer.get());
    if (proxyPeer != nullptr)
    {
        _peerToProxyPeers[proxyPeer->GetPeer()].insert(proxyPeer);
    }

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
    if (!_isShuttingDown)
    {
        std::vector<IVAsioPeer*> proxyPeers;

        {
            std::unique_lock<std::mutex> lock{_peersLock};

            const auto it = _peerToProxyPeers.find(peer);
            if (it != _peerToProxyPeers.end())
            {
                std::copy(it->second.begin(), it->second.end(), std::back_inserter(proxyPeers));
                _peerToProxyPeers.erase(it);
            }
        }

        for (IVAsioPeer* const proxyPeer : proxyPeers)
        {
            OnPeerShutdown(proxyPeer);
        }

        {
            std::unique_lock<std::mutex> lock{_peersLock};

            SendProxyPeerShutdownNotification(peer);

            for (auto&& callback : _peerShutdownCallbacks)
            {
                callback(peer);
            }

            RemovePeerFromLinks(peer);
            RemovePeerFromConnection(peer);
        }
    }
}

void VAsioConnection::SendProxyPeerShutdownNotification(IVAsioPeer* peer)
{
    const auto & source = peer->GetInfo().participantName;

    const auto proxyDestinationsIt = _proxySourceToDestinations.find(source);
    if (proxyDestinationsIt != _proxySourceToDestinations.end())
    {
        for (const auto& destination : proxyDestinationsIt->second)
        {
            // if a destination participant name has no associated peer, ignore it, it was already been disconnected
            const auto peerIt = _participantNameToPeer.find(destination);
            if (peerIt == _participantNameToPeer.end())
            {
                continue;
            }

            ProxyMessage msg{};
            msg.source = source;
            msg.destination = destination;
            msg.payload.clear();

            peerIt->second->SendSilKitMsg(SerializedMessage{std::move(msg)});
        }
    }
}

void VAsioConnection::RemovePeerFromLinks(IVAsioPeer* peer)
{
    tt::for_each(_links, [this, peer](auto&& linkMap) {
        std::unique_lock<decltype(_linksMx)> lock{_linksMx};

        // remove the peer from the links without taking the lock
        for (auto& kv : linkMap)
        {
            kv.second->RemoveRemoteReceiver(peer);
        }
    });
}

void VAsioConnection::RemovePeerFromConnection(IVAsioPeer* peer)
{
    _participantNameToPeer.erase(peer->GetInfo().participantName);

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

void VAsioConnection::HandleExpiredConnection(const asio::error_code& ec)
{
    if (ec)
    {
        return;
    }

    SilKit::Services::Logging::Debug(_logger, "Remote connection time out reached. Number of remote connections: {}", _pendingRemoteConnections.size());

    auto now = std::chrono::steady_clock::now();
    std::unique_lock<decltype(_pendingRemoteMx)> lock(_pendingRemoteMx);
    std::unique_lock<decltype(_peersLock)> lock2(_peersLock);
    //clean up successful connections
    for (auto&& existingPeer : _peers)
    {
        auto&& participantName = existingPeer->GetInfo().participantName;
        auto it = _pendingRemoteConnections.find(participantName);
        if (it != _pendingRemoteConnections.end())
        {
            SilKit::Services::Logging::Trace(_logger, "Remote connection to '{}' succeeded", participantName);
            _pendingRemoteConnections.erase(it);
        }
    }

    for (auto it = _pendingRemoteConnections.begin(); it != _pendingRemoteConnections.end(); /*nop */)
    {
        auto&& pendingParticipant = it->first;
        auto&& pendingRemoteConn = it->second;

        if (now >= pendingRemoteConn.timer->expiry())
        {
            SilKit::Services::Logging::Trace(_logger, "Dropping pending remote connection to '{}'", pendingParticipant);
            it = _pendingRemoteConnections.erase(it++);
        }
        else
        {
            ++it;
        }
    }

}

void VAsioConnection::RemovePeerFromPendingLists(IVAsioPeer* from)
{
    auto iter = std::find_if(_pendingParticipantReplies.begin(), _pendingParticipantReplies.end(),
            [&from](const auto& peer) {
                return peer->GetInfo().participantName == from->GetInfo().participantName;
        });
    if (iter != _pendingParticipantReplies.end())
    {
        _pendingParticipantReplies.erase(iter);
        if (_pendingParticipantReplies.empty())
        {
            try
            {
                _receivedAllParticipantReplies.set_value();
            }
            catch (...)
            {
            }
        }
    }

    if (_participantId != RegistryParticipantId)
    {
        // remove pending remote connection 
        std::unique_lock<decltype(_pendingRemoteMx)> lock(_pendingRemoteMx);
        auto pendingIt = _pendingRemoteConnections.find(from->GetInfo().participantName);
        if (pendingIt != _pendingRemoteConnections.end())
        {
            // finish our initial connect attempt, by sending our ParticipantAnnouncement over the
            // established remote connection.
            SendParticipantAnnouncement(from);
            Services::Logging::Debug(_logger, "Remote connection to '{}' succeeded.", from->GetInfo().participantName);
            _pendingRemoteConnections.erase(pendingIt);
        }
    }
}

void VAsioConnection::NotifyShutdown()
{
    _isShuttingDown = true;
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
    case VAsioMsgKind::SilKitProxyMessage:
        return ReceiveProxyMessage(from, std::move(buffer));
    }
}

void VAsioConnection::ReceiveProxyMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    const auto proxyMessageHeader = buffer.GetProxyMessageHeader();
    if (proxyMessageHeader.version != 0)
    {
        static SilKit::Services::Logging::LogOnceFlag onceFlag;
        SilKit::Services::Logging::Warn(
            _logger, onceFlag,
            "Ignoring VAsioMsgKind::SilKitProxyMessage because message version is not supported: version {}",
            proxyMessageHeader.version);
        return;
    }

    auto proxyMessage = buffer.Deserialize<ProxyMessage>();

    if (!_config.middleware.registryAsFallbackProxy)
    {
        static SilKit::Services::Logging::LogOnceFlag onceFlag;
        SilKit::Services::Logging::Warn(
            _logger, onceFlag,
            "Ignoring VAsioMsgKind::SilKitProxyMessage because feature is disabled via configuration: From {}, To {}",
            proxyMessage.source, proxyMessage.destination);
        return;
    }

    SilKit::Services::Logging::Trace(_logger,
                                     "Received message with VAsioMsgKind::SilKitProxyMessage: From {}, To {}",
                                     proxyMessage.source, proxyMessage.destination);

    const bool fromIsSource = from->GetInfo().participantName == proxyMessage.source;
    if (fromIsSource)
    {
        auto it = _participantNameToPeer.find(proxyMessage.destination);
        if (it == _participantNameToPeer.end())
        {
            SilKit::Services::Logging::Error(_logger, "Unable to deliver proxy message from {} to {}",
                                             proxyMessage.source, proxyMessage.destination);
            return;
        }

        it->second->SendSilKitMsg(SerializedMessage{proxyMessage});

        // We are relaying a message from source to destination and acting as a proxy. Record the association between
        // source and destination. This is used during disconnects, where we create empty ProxyMessages on behalf of
        // the disconnected peer, to inform the destination that the source peer has disconnected.
        _proxySourceToDestinations[proxyMessage.source].insert(proxyMessage.destination);

        return;
    }

    const bool isDestination = GetParticipantName() == proxyMessage.destination;
    if (isDestination)
    {
        auto it = _participantNameToPeer.find(proxyMessage.source);

        IVAsioPeer* peer{nullptr};

        if (it == _participantNameToPeer.end())
        {
            SilKit::Services::Logging::Debug(_logger, "Creating VAsioProxyPeer ({})", proxyMessage.source);

            auto proxyPeer = std::make_shared<VAsioProxyPeer>(this, VAsioPeerInfo{}, from, _logger);
            AddPeer(proxyPeer);

            peer = proxyPeer.get();
        }
        else
        {
            peer = it->second;
        }

        // An empty payload signals shutdown of the proxied peer.
        if (proxyMessage.payload.empty())
        {
            OnPeerShutdown(peer);
        }
        else
        {
            OnSocketData(peer, SerializedMessage{std::move(proxyMessage.payload)});
        }

        return;
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
        Services::Logging::Warn(_logger,
            "Received SubscriptionAnnouncement from {} for message type {}"
            " for an unknown subscriber version {}",
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
        Services::Logging::Error(_logger, "Failed to subscribe [{}] {} from {}"
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

    tt::for_each(_links, [this, &from, &subscriber, &wasAdded](auto&& linkMap) {
        using LinkPtr = typename std::decay_t<decltype(linkMap)>::mapped_type;
        using LinkType = typename LinkPtr::element_type;

        if (subscriber.msgTypeName != LinkType::MessageSerdesName())
            return;

        // create the link under lock
        std::unique_lock<decltype(_linksMx)> lock{_linksMx};
        auto& link = linkMap[subscriber.networkName];
        if (!link)
        {
            link = std::make_shared<LinkType>(subscriber.networkName, _logger, _timeProvider);
        }
        lock.unlock();

        // add the remote receiver without taking the lock
        link->AddRemoteReceiver(from, subscriber.receiverIdx);

        wasAdded = true;
    });

    if (wasAdded)
    {
        Services::Logging::Debug(_logger, "Messages of type '{}' on link '{}' will be sent to participant '{}'",
                                 from->GetInfo().participantName, subscriber.msgTypeName, subscriber.networkName);
    }
    else
    {
        Services::Logging::Warn(_logger, "Participant '{}' could not be registered as receiver for messages of type '{}' on link '{}'",
                                from->GetInfo().participantName, subscriber.msgTypeName, subscriber.networkName);
    }

    return wasAdded;
}

void VAsioConnection::ReceiveRawSilKitMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto receiverIdx = static_cast<size_t>(buffer.GetRemoteIndex());//ExtractEndpointId(buffer);
    if (receiverIdx >= _vasioReceivers.size())
    {
        Services::Logging::Warn(_logger, "Ignoring RawSilKitMessage for unknown receiverIdx={}", receiverIdx);
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
    const auto header = buffer.GetRegistryMessageHeader();
    if (header.preamble != REGISTRY_MESSAGE_HEADER_PREAMBLE_VALUE)
    {
        Services::Logging::Warn(_logger, "Ignoring registry message from '{}' with invalid preamble {}",
                                from->GetInfo().participantName, header.preamble);
        return;
    }

    const auto kind = buffer.GetRegistryKind();
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
    case RegistryMessageKind::RemoteParticipantConnectRequest:
        return ReceiveRemoteParticipantConnectRequest(std::move(buffer));
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

auto VAsioConnection::GetNumberOfRemoteReceivers(const IServiceEndpoint* service, const std::string& msgTypeName)
    -> size_t
{
    auto networkName = service->GetServiceDescriptor().GetNetworkName();

    size_t result = 0;
    tt::for_each(_links, [this, &networkName, msgTypeName, &result](auto&& linkMap) {
        using LinkPtr = typename std::decay_t<decltype(linkMap)>::mapped_type;
        using LinkType = typename LinkPtr::element_type;

        if (result != 0)
            return;

        if (msgTypeName != LinkType::MessageSerdesName())
            return;

        // access the link under lock
        std::unique_lock<decltype(_linksMx)> lock{_linksMx};
        auto& link = linkMap[networkName];
        if (link)
        {
            result = link->GetNumberOfRemoteReceivers();
        }
    });
    
    return result;
}

auto VAsioConnection::GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* service,
                                                           const std::string& msgTypeName)
    -> std::vector<std::string>
{
    auto networkName = service->GetServiceDescriptor().GetNetworkName();

    std::vector<std::string> result{};
    tt::for_each(_links, [this, &networkName, msgTypeName, &result](auto&& linkMap) {
        using LinkPtr = typename std::decay_t<decltype(linkMap)>::mapped_type;
        using LinkType = typename LinkPtr::element_type;

        if (!result.empty())
            return;

        if (msgTypeName != LinkType::MessageSerdesName())
            return;

        // access the link under lock
        std::unique_lock<decltype(_linksMx)> lock{_linksMx};
        auto& link = linkMap[networkName];
        if (link)
        {
            result = link->GetParticipantNamesOfRemoteReceivers();
        }
    });

    return result;
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

bool VAsioConnection::ParticiantHasCapability(const std::string& participantName, const std::string& capability) const
{
    const auto it = _participantNameToPeer.find(participantName);
    if (it == _participantNameToPeer.end())
    {
        SilKit::Services::Logging::Warn(_logger, "GetParticiantCapabilities: Participant '{}' unknown",
                                        participantName);
        return false;
    }
    else
    {
        auto capabilities = SilKit::Core::VAsioCapabilities{it->second->GetInfo().capabilities};
        return capabilities.HasCapability(capability);
    }
}

} // namespace Core
} // namespace SilKit
