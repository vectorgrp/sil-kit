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

#include "ILoggerInternal.hpp"
#include "VAsioConstants.hpp"
#include "VAsioPeer.hpp"
#include "VAsioProxyPeer.hpp"
#include "Filesystem.hpp"
#include "SetThreadName.hpp"
#include "Uri.hpp"
#include "Assert.hpp"
#include "TransformAcceptorUris.hpp"

#include "ConnectPeer.hpp"
#include "util/TracingMacros.hpp"

#include "asio.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioConnection
#define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#define SILKIT_TRACE_METHOD_(...)
#endif


namespace Log = SilKit::Services::Logging;


using namespace std::chrono_literals;
namespace fs = SilKit::Filesystem;

namespace {

auto printableName(const std::string& participantName) -> std::string
{
    std::string safeName;
    for (const auto& ch : participantName)
    {
        // do not use std::isalnum, as it may sensitive to the current locale
        const bool isAlphaNumeric{('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9')
                                  || (ch == '_' || ch == '-' || ch == '.' || ch == '~')};

        if (isAlphaNumeric)
        {
            safeName.push_back(ch);
        }
        else
        {
            safeName += fmt::format("{:02X}", static_cast<unsigned char>(ch));
        }
    }
    return safeName;
}

//Debug  print of given peer infos

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
    const auto bounded_name = safe_name.substr(0, std::min<size_t>(safe_name.size(), 10));

    // We hash the participant name, ID and the current working directory
    // as part of the temporary file name, so we can have multiple local simulations
    // started from different working directories, but a shared temporary directory.
    // NB keep the variable part as short as possible.

    const auto unique_id =
        std::hash<std::string>{}(participantName + std::to_string(id) + uniqueValue + fs::current_path().string());

    std::stringstream path;
    path << fs::temp_directory_path().string() << fs::path::preferred_separator << bounded_name << std::hex << unique_id
         << ".silkit";

    result.path(path.str());
    return result;
}

// end point to string conversions
auto fromAsioEndpoint(const asio::local::stream_protocol::endpoint& ep)
{
    std::stringstream uri;
    const std::string localPrefix{"local://"};
    uri << localPrefix << ep.path();
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

auto selectBestEndpointFromResolverResults(const std::vector<asio::ip::tcp::endpoint>& resolverResults)
    -> asio::ip::tcp::endpoint
{
    // NB: IPv4 should be preferred over IPv6

    std::multimap<int, asio::ip::tcp::endpoint> endpointsByPenalty;

    for (const auto& resolvedEndpoint : resolverResults)
    {
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


auto MakeCapabilitiesFromConfiguration(const SilKit::Config::ParticipantConfiguration& participantConfiguration)
    -> SilKit::Core::VAsioCapabilities
{
    SilKit::Core::VAsioCapabilities capabilities;

    capabilities.AddCapability(SilKit::Core::Capabilities::AutonomousSynchronous);

    if (participantConfiguration.middleware.registryAsFallbackProxy)
    {
        capabilities.AddCapability(SilKit::Core::Capabilities::ProxyMessage);
    }

    if (participantConfiguration.middleware.experimentalRemoteParticipantConnection)
    {
        capabilities.AddCapability(SilKit::Core::Capabilities::RequestParticipantConnection);
    }

    return capabilities;
}

auto MakeCapabilitiesStringFromConfiguration(const SilKit::Config::ParticipantConfiguration& participantConfiguration)
    -> std::string
{
    auto capabilities{MakeCapabilitiesFromConfiguration(participantConfiguration)};
    return capabilities.ToCapabilitiesString();
}


auto MakeAsioSocketOptionsFromConfiguration(const SilKit::Config::ParticipantConfiguration& participantConfiguration)
    -> SilKit::Core::AsioSocketOptions
{
    SilKit::Core::AsioSocketOptions socketOptions{};
    socketOptions.tcp.quickAck = participantConfiguration.middleware.tcpQuickAck;
    socketOptions.tcp.noDelay = participantConfiguration.middleware.tcpNoDelay;
    socketOptions.tcp.sendBufferSize = participantConfiguration.middleware.tcpSendBufferSize;
    socketOptions.tcp.receiveBufferSize = participantConfiguration.middleware.tcpReceiveBufferSize;

    return socketOptions;
}


auto GetConnectTimeoutSeconds(const SilKit::Config::ParticipantConfiguration& config) -> std::chrono::milliseconds
{
    std::chrono::duration<double> seconds{config.middleware.connectTimeoutSeconds};
    auto milliseconds{std::chrono::duration_cast<std::chrono::milliseconds>(seconds)};
    return std::max(100ms, milliseconds);
}


auto GetRegistryConnectTimeout(const SilKit::Config::ParticipantConfiguration& config) -> std::chrono::milliseconds
{
    return GetConnectTimeoutSeconds(config);
}

auto GetRegistryHandshakeTimeout(const SilKit::Config::ParticipantConfiguration& config) -> std::chrono::milliseconds
{
    return GetConnectTimeoutSeconds(config);
}

auto GetParticipantHandshakeTimeout(const SilKit::Config::ParticipantConfiguration& config) -> std::chrono::milliseconds
{
    return GetConnectTimeoutSeconds(config);
}

auto MakeConnectKnownParticipantsSettings(const SilKit::Config::ParticipantConfiguration& config)
    -> SilKit::Core::ConnectKnownParticipantsSettings
{
    SilKit::Core::ConnectKnownParticipantsSettings settings;
    settings.directConnectTimeout = GetConnectTimeoutSeconds(config);
    settings.remoteConnectRequestTimeout = GetConnectTimeoutSeconds(config);
    return settings;
}

auto MakeRemoteConnectionManagerSettings(const SilKit::Config::ParticipantConfiguration& config)
    -> SilKit::Core::RemoteConnectionManagerSettings
{
    SilKit::Core::RemoteConnectionManagerSettings settings;
    settings.connectTimeout = GetConnectTimeoutSeconds(config);
    return settings;
}


} // namespace


namespace SilKit {
namespace Core {

namespace tt = Util::tuple_tools;

VAsioConnection::VAsioConnection(IParticipantInternal* participant, IMetricsManager* metricsManager,
                                 SilKit::Config::ParticipantConfiguration config, std::string participantName,
                                 ParticipantId participantId, Services::Orchestration::ITimeProvider* timeProvider,
                                 ProtocolVersion version)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
    , _timeProvider{timeProvider}
    , _capabilities{MakeCapabilitiesFromConfiguration(_config)}
    , _ioContext{MakeAsioIoContext(MakeAsioSocketOptionsFromConfiguration(_config))}
    , _connectKnownParticipants{*_ioContext, *this, *this, MakeConnectKnownParticipantsSettings(_config)}
    , _remoteConnectionManager{*this, MakeRemoteConnectionManagerSettings(_config)}
    , _version{version}
    , _metricsManager{metricsManager}
    , _participant{participant}
{
}

VAsioConnection::~VAsioConnection()
{
    _isShuttingDown = true;

    _ioContext->Post([this] {
        {
            std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};

            for (const auto& acceptor : _acceptors)
            {
                acceptor->Shutdown();
            }
        }

        {
            std::unique_lock<decltype(_peersLock)> lock{_peersLock};

            for (const auto& peer : _peers)
            {
                peer->Shutdown();
            }
        }

        _connectKnownParticipants.Shutdown();
        _remoteConnectionManager.Shutdown();

        if (_registry != nullptr)
        {
            _registry->Shutdown();
        }
    });

    StartIoWorker();

    if (_ioWorker.joinable())
    {
        _ioWorker.join();
    }
}

void VAsioConnection::SetLogger(Services::Logging::ILogger* logger)
{
    _logger = logger;

    _ioContext->SetLogger(*_logger);
    _connectKnownParticipants.SetLogger(*_logger);
}

auto VAsioConnection::GetLogger() -> SilKit::Services::Logging::ILogger*
{
    return _logger;
}

auto VAsioConnection::PrepareAcceptorEndpointUris(const std::string& connectUri) -> std::vector<std::string>
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

void VAsioConnection::OpenTcpAcceptors(const std::vector<std::string>& acceptorEndpointUris)
{
    auto metric = _metricsManager->GetStringList("TcpAcceptors");
    metric->Clear();

    for (const auto& uriString : acceptorEndpointUris)
    {
        const auto uri = Uri::Parse(uriString);

        if (uri.Type() == Uri::UriType::Tcp && uri.Scheme() == "tcp")
        {
            SilKit::Services::Logging::Debug(_logger, "Found TCP acceptor endpoint URI {} with host {} and port {}",
                                             uriString, uri.Host(), uri.Port());

            for (const auto& host : _ioContext->Resolve(uri.Host()))
            {
                Services::Logging::Debug(_logger, "Accepting TCP connections on {}:{}", host, uri.Port());

                try
                {
                    auto acceptor{_ioContext->MakeTcpAcceptor(host, uri.Port())};
                    acceptor->SetListener(*this);
                    acceptor->AsyncAccept({});

                    {
                        std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};
                        _acceptors.emplace_back(std::move(acceptor));
                    }
                }
                catch (const std::exception& exception)
                {
                    Services::Logging::Error(_logger, "Unable to accept TCP connections on {}:{}: {}", host, uri.Port(),
                                             exception.what());
                }

                metric->Add(fmt::format("{}:{}", host, uri.Port()));
            }
        }
        else if (uri.Type() == Uri::UriType::Local && uri.Scheme() == "local")
        {
            // do nothing, handled elsewhere
        }
        else
        {
            SilKit::Services::Logging::Warn(_logger, "OpenTcpAcceptors: Unused acceptor endpoint URI: {}", uriString);
        }
    }
}

void VAsioConnection::OpenLocalAcceptors(const std::vector<std::string>& acceptorEndpointUris)
{
    auto metric = _metricsManager->GetStringList("LocalAcceptors");
    metric->Clear();

    for (const auto& uriString : acceptorEndpointUris)
    {
        const auto uri = Uri::Parse(uriString);

        if (uri.Type() == Uri::UriType::Local && uri.Scheme() == "local")
        {
            SilKit::Services::Logging::Debug(_logger, "Found local domain acceptor endpoint URI {} with path {}",
                                             uriString, uri.Path());

            // file must not exist before we bind/listen on it
            (void)fs::remove(uri.Path());

            try
            {
                auto acceptor{_ioContext->MakeLocalAcceptor(uri.Path())};
                acceptor->SetListener(*this);
                acceptor->AsyncAccept({});

                {
                    std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};
                    _acceptors.emplace_back(std::move(acceptor));
                }
            }
            catch (const std::exception& exception)
            {
                Services::Logging::Error(_logger, "Unable to accept local domain connections on '{}': {}", uri.Path(),
                                         exception.what());
            }

            metric->Add(fmt::format("{}", uri.Path()));
        }
        else if (uri.Type() == Uri::UriType::Tcp && uri.Scheme() == "tcp")
        {
            // do nothing, handled elsewhere
        }
        else
        {
            SilKit::Services::Logging::Warn(_logger, "OpenLocalAcceptors: Unused acceptor endpoint URI: {}", uriString);
        }
    }
}

void VAsioConnection::JoinSimulation(std::string connectUri)
{
    SILKIT_ASSERT(_logger);

    _simulationName = Uri{connectUri}.Path();
    _allowAnySimulationName = false;

    // Open all configured acceptors and start accepting connections.
    OpenParticipantAcceptors(connectUri);

    // Connects this participant to the registry. Each connection attempt has its own timeout.
    ConnectParticipantToRegistryAndStartIoWorker(connectUri);

    // Wait for a fixed amount of time for the registry connection to complete.
    WaitForRegistryHandshakeToComplete(GetRegistryHandshakeTimeout(_config));

    // Start connecting and initiate the handshakes with all known participants.
    ConnectToKnownParticipants();

    // Wait for a fixed amount of time for all handshakes to complete.
    WaitForAllReplies(GetParticipantHandshakeTimeout(_config));

    _logger->Debug("Connected to all known participants");
}

void VAsioConnection::OpenParticipantAcceptors(const std::string& connectUri)
{
    const auto acceptorEndpointUris = PrepareAcceptorEndpointUris(connectUri);

    if (_config.middleware.enableDomainSockets)
    {
        OpenLocalAcceptors(acceptorEndpointUris);
    }

    // Accept TCP connections on endpoints given by matching URIs
    OpenTcpAcceptors(acceptorEndpointUris);

    if (_acceptors.empty())
    {
        SilKit::Services::Logging::Error(_logger, "JoinSimulation: no acceptors available");
        throw SilKitError{"JoinSimulation: no acceptors available"};
    }
}

void VAsioConnection::ConnectParticipantToRegistryAndStartIoWorker(const std::string& connectUriString)
{
    _logger->Debug("Connecting to SIL Kit Registry");

    VAsioPeerInfo registryPeerInfo;
    registryPeerInfo.participantName = REGISTRY_PARTICIPANT_NAME;
    registryPeerInfo.participantId = REGISTRY_PARTICIPANT_ID;

    if (_config.middleware.enableDomainSockets)
    {
        auto pi = makeLocalPeerInfo(REGISTRY_PARTICIPANT_NAME, REGISTRY_PARTICIPANT_ID, connectUriString);
        std::copy(pi.acceptorUris.begin(), pi.acceptorUris.end(), std::back_inserter(registryPeerInfo.acceptorUris));
    }

    Uri connectUri{connectUriString};
    registryPeerInfo.acceptorUris.emplace_back(Uri::MakeTcp(connectUri.Host(), connectUri.Port()).EncodedString());

    struct ConnectRegistryCallbacks final : IConnectPeerListener
    {
        std::promise<std::unique_ptr<IRawByteStream>> promise;

        void OnConnectPeerSuccess(IConnectPeer&, VAsioPeerInfo, std::unique_ptr<IRawByteStream> stream) override
        {
            promise.set_value(std::move(stream));
        }

        void OnConnectPeerFailure(IConnectPeer&, VAsioPeerInfo) override
        {
            promise.set_value(nullptr);
        }
    };

    ConnectRegistryCallbacks connectRegistryCallbacks;
    auto registryStreamFuture{connectRegistryCallbacks.promise.get_future()};

    auto connectRegistry{MakeConnectPeer(registryPeerInfo)};
    connectRegistry->SetListener(connectRegistryCallbacks);
    connectRegistry->AsyncConnect(_config.middleware.connectAttempts, GetRegistryConnectTimeout(_config));

    StartIoWorker();

    auto registryStream{registryStreamFuture.get()};
    if (registryStream == nullptr)
    {
        Services::Logging::Error(_logger, "Failed to connect to SIL Kit Registry (number of attempts: {})",
                                 _config.middleware.connectAttempts);

        Services::Logging::Info(
            _logger,
            "   Make sure that the SIL Kit Registry is up and running and is listening on the following URIs: {}.",
            printUris(registryPeerInfo.acceptorUris));
        _logger->Info("   If a registry is unable to open a listening socket it will only be reachable"
                      " via local domain sockets, which depend on the working directory"
                      " and the middleware configuration ('enableDomainSockets').");
        _logger->Info("   Make sure that the hostname can be resolved and is reachable.");
        _logger->Info("   You can configure the SIL Kit Registry hostname and port via the SilKitConfig.");
        _logger->Info("   The SIL Kit Registry executable can be found in your SIL Kit installation folder:");
        _logger->Info("     INSTALL_DIR/bin/sil-kit-registry[.exe]");
        throw SilKitError{"ERROR: Failed to connect to SIL Kit Registry"};
    }

    _registry = MakeVAsioPeer(std::move(registryStream));
    _registry->SetInfo(registryPeerInfo);

    SilKit::Services::Logging::Info(_logger, "Connected to registry at '{}' via '{}' ({})",
                                    _registry->GetRemoteAddress(), _registry->GetLocalAddress(),
                                    printUris(_registry->GetInfo().acceptorUris));

    _registry->StartAsyncRead();

    SendParticipantAnnouncement(_registry.get());
}

void VAsioConnection::WaitForRegistryHandshakeToComplete(std::chrono::milliseconds timeout)
{
    SilKit::Services::Logging::Debug(_logger, "Waiting {}ms for the registry to reply", timeout.count());

    auto future{_registryHandshakeComplete.get_future()};
    auto futureStatus{future.wait_for(timeout)};

    if (futureStatus == std::future_status::timeout)
    {
        std::string errorMessage{fmt::format(
            "Timeout during connection handshake with the SIL Kit Registry. This might indicate that a participant "
            "with the same name ('{}') has already connected to the registry.",
            _participantName)};

        _logger->Error(errorMessage);
        throw SilKit::ProtocolError{errorMessage};
    }

    SILKIT_ASSERT(futureStatus == std::future_status::ready);

    // propagate any exception stored in the promise
    future.get();
}

void VAsioConnection::ConnectToKnownParticipants()
{
    _logger->Debug("Connecting to known participants");

    _connectKnownParticipants.StartConnecting();

    auto future{_startWaitingForParticipantHandshakes.get_future()};

    // propagate any exception stored in the promise
    future.get();
}

void VAsioConnection::WaitForAllReplies(std::chrono::milliseconds timeout)
{
    SilKit::Services::Logging::Debug(_logger, "Waiting {}ms for all known participants to reply", timeout.count());

    auto future{_allKnownParticipantHandshakesComplete.get_future()};
    auto futureStatus{future.wait_for(timeout)};

    if (futureStatus == std::future_status::timeout)
    {
        std::string errorMessage{fmt::format("Timeout while waiting for replies from known participants: {}",
                                             _connectKnownParticipants.Describe())};

        _logger->Error(errorMessage);
        throw SilKit::ProtocolError{errorMessage};
    }

    SILKIT_ASSERT(futureStatus == std::future_status::ready);

    // propagate any exception stored in the promise
    future.get();
}


void VAsioConnection::LogAndPrintNetworkIncompatibility(const RegistryMsgHeader& other,
                                                        const std::string& otherParticipantName)
{
    const auto errorMsg = fmt::format("Network incompatibility between this version range ({})"
                                      " and connecting participant '{}' ({})",
                                      MapVersionToRelease(MakeRegistryMsgHeader(_version)), otherParticipantName,
                                      MapVersionToRelease(other));
    _logger->Critical(errorMsg);
    std::cerr << "ERROR: " << errorMsg << std::endl;
}

void VAsioConnection::SendParticipantAnnouncement(IVAsioPeer* peer)
{
    auto myPeerInfo{MakePeerInfo()};

    for (const auto& uri : myPeerInfo.acceptorUris)
    {
        Services::Logging::Trace(_logger, "SendParticipantAnnouncement: Peer '{}' ('{}'): Acceptor Uri: {}",
                                 peer->GetInfo().participantName, peer->GetSimulationName(), uri);
    }

    if (myPeerInfo.acceptorUris.empty())
    {
        const auto message = "SendParticipantAnnouncement: Cannot send announcement: All acceptors "
                             "(both Local-Domain and TCP) are missing";
        SilKit::Services::Logging::Error(_logger, message);
        throw SilKitError{message};
    }

    ParticipantAnnouncement announcement;
    // We override the default protocol version here for integration testing
    announcement.messageHeader = MakeRegistryMsgHeader(_version);
    announcement.peerInfo = std::move(myPeerInfo);
    announcement.simulationName = _simulationName;

    Services::Logging::Debug(_logger, "Sending participant announcement to '{}' ('{}')",
                             peer->GetInfo().participantName, peer->GetSimulationName());
    peer->SendSilKitMsg(SerializedMessage{announcement});
}

void VAsioConnection::ReceiveParticipantAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer)
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    const auto remoteHeader = buffer.GetRegistryMessageHeader();

    // check if we support the remote peer's protocol version or signal a handshake failure
    if (ProtocolVersionSupported(remoteHeader))
    {
        from->SetProtocolVersion(ExtractProtocolVersion(remoteHeader));
    }
    else
    {
        // it is not safe to decode the ParticipantAnnouncement
        LogAndPrintNetworkIncompatibility(remoteHeader, from->GetRemoteAddress());

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

    Services::Logging::Debug(_logger,
                             "Received participant announcement from {}, protocol version {}.{}, simulation name '{}'",
                             announcement.peerInfo.participantName, announcement.messageHeader.versionHigh,
                             announcement.messageHeader.versionLow, announcement.simulationName);

    if (!_allowAnySimulationName && announcement.simulationName != _simulationName)
    {
        SendFailedParticipantAnnouncementReply(
            from,
            // tell the remote peer what protocol version we accepted for communication with them
            from->GetProtocolVersion(),
            // use the exception message as the diagnostic
            fmt::format("Announced simulation name '{}' does not match this participants simulation name '{}'",
                        announcement.simulationName, _simulationName));

        return;
    }

    from->SetInfo(announcement.peerInfo);
    auto& service = dynamic_cast<IServiceEndpoint&>(*from);
    auto serviceDescriptor = service.GetServiceDescriptor();
    serviceDescriptor.SetParticipantNameAndComputeId(announcement.peerInfo.participantName);
    service.SetServiceDescriptor(serviceDescriptor);

    from->SetSimulationName(announcement.simulationName);

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

    AssociateParticipantNameAndPeer(announcement.simulationName, announcement.peerInfo.participantName, from);
    SendParticipantAnnouncementReply(from);
}

void VAsioConnection::SendParticipantAnnouncementReply(IVAsioPeer* peer)
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    ParticipantAnnouncementReply reply;
    // tell the remote peer what *our* protocol version is that we can accept for this peer
    reply.remoteHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());
    reply.status = ParticipantAnnouncementReply::Status::Success;
    // fill in the service descriptors we want to subscribe to
    std::transform(_vasioReceivers.begin(), _vasioReceivers.end(), std::back_inserter(reply.subscribers),
                   [](const auto& subscriber) { return subscriber->GetDescriptor(); });

    Services::Logging::Debug(_logger, "Sending ParticipantAnnouncementReply to '{}' ('{}') with protocol version {}",
                             peer->GetInfo().participantName, peer->GetSimulationName(),
                             ExtractProtocolVersion(reply.remoteHeader));

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::SendFailedParticipantAnnouncementReply(IVAsioPeer* peer, ProtocolVersion version,
                                                             std::string diagnostic)
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    ParticipantAnnouncementReply reply;
    reply.remoteHeader = MakeRegistryMsgHeader(version);
    reply.status = ParticipantAnnouncementReply::Status::Failed;
    reply.diagnostic = std::move(diagnostic);

    Services::Logging::Debug(
        _logger, "Sending failed ParticipantAnnouncementReply to '{}' ('{}') with protocol version {}",
        peer->GetInfo().participantName, peer->GetSimulationName(), ExtractProtocolVersion(reply.remoteHeader));

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});
}

void VAsioConnection::ReceiveParticipantAnnouncementReply(IVAsioPeer* from, SerializedMessage&& buffer)
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    auto reply = buffer.Deserialize<ParticipantAnnouncementReply>();
    const auto& remoteVersion = ExtractProtocolVersion(reply.remoteHeader);

    if (reply.status == ParticipantAnnouncementReply::Status::Failed)
    {
        const auto message = fmt::format(
            "SIL Kit Connection Handshake: Received failed ParticipantAnnouncementReply from '{}' ('{}') with "
            "protocol version {} and diagnostic message: {}",
            from->GetInfo().participantName, from->GetSimulationName(),
            // extract the version delivered in the reply (
            ExtractProtocolVersion(reply.remoteHeader),
            // provide a non-empty default string for the diagnostic message
            reply.diagnostic.empty() ? "(no diagnostic message was delivered)" : reply.diagnostic.c_str());

        _logger->Warn(message);

        // tear down the participant if we are talking to the registry
        if (from->GetInfo().participantId == REGISTRY_PARTICIPANT_ID)
        {
            // handshake with SIL Kit Registry failed, we need to abort.
            auto error = ProtocolError(message);

            // propagate the error to the main thread
            _registryHandshakeComplete.set_exception(std::make_exception_ptr(error));

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

    Services::Logging::Debug(_logger, "Received participant announcement reply from '{}' ('{}') protocol version {}",
                             from->GetInfo().participantName, from->GetSimulationName(), remoteVersion);

    if (from->GetInfo().participantId == REGISTRY_PARTICIPANT_ID)
    {
        _registryHandshakeComplete.set_value();
    }
    else
    {
        _connectKnownParticipants.HandlePeerEvent(from->GetInfo().participantName,
                                                  ConnectKnownParticipants::PeerEvent::PARTICIPANT_ANNOUNCEMENT_REPLY);
    }
}

void VAsioConnection::ReceiveKnownParticpants(IVAsioPeer* peer, SerializedMessage&& buffer)
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    auto msg = buffer.Deserialize<KnownParticipants>();

    Services::Logging::Debug(_logger, "Received known participants list from '{}' ('{}') protocol {}.{}",
                             peer->GetInfo().participantName, peer->GetSimulationName(), msg.messageHeader.versionHigh,
                             msg.messageHeader.versionLow);

    // After receiving a ParticipantAnnouncement the Registry will send a KnownParticipants message
    // check if we support its version here
    if (!ProtocolVersionSupported(msg.messageHeader))
    {
        // Not acknowledged
        LogAndPrintNetworkIncompatibility(msg.messageHeader, peer->GetInfo().participantName);

        ParticipantAnnouncementReply reply;
        reply.status = ParticipantAnnouncementReply::Status::Failed;
        reply.remoteHeader = MakeRegistryMsgHeader(_version);

        peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), reply});

        return;
    }

    peer->SetProtocolVersion(ExtractProtocolVersion(msg.messageHeader));

    _connectKnownParticipants.SetKnownParticipants(msg.peerInfos);
}


void VAsioConnection::ReceiveRemoteParticipantConnectRequest(IVAsioPeer* peer, SerializedMessage&& buffer)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");

    const auto registryMsgHeader = buffer.GetRegistryMessageHeader();

    // check if we support the remote peer's protocol version or signal a handshake failure
    if (ProtocolVersionSupported(registryMsgHeader))
    {
        if (peer->GetInfo().participantId != REGISTRY_PARTICIPANT_ID)
        {
            peer->SetProtocolVersion(ExtractProtocolVersion(registryMsgHeader));
        }
    }
    else
    {
        Log::Warn(_logger,
                  "Received RemoteParticipantConnectRequest from peer '{}' ('{}' at '{}') with unsupported protocol "
                  "version {}.{}",
                  peer->GetInfo().participantName, peer->GetSimulationName(), peer->GetRemoteAddress(),
                  registryMsgHeader.versionHigh, registryMsgHeader.versionLow);

        // ignore the request if it is sent via the registry (we cannot deserialize it anyway)
        if (peer->GetInfo().participantId == REGISTRY_PARTICIPANT_ID)
        {
            Log::Debug(_logger, "Dropping invalid RemoteParticipantConnectRequest from registry");
            return;
        }

        // it is not safe to decode the RemoteParticipantConnectRequest
        LogAndPrintNetworkIncompatibility(registryMsgHeader, peer->GetRemoteAddress());

        return;
    }

    auto msg{buffer.Deserialize<RemoteParticipantConnectRequest>()};

    if (!_capabilities.HasRequestParticipantConnectionCapability())
    {
        SilKit::Services::Logging::Warn(_logger,
                                        "Ignoring RemoteParticipantConnectRequest because feature is disabled via "
                                        "configuration: origin {}, target {}",
                                        msg.requestOrigin.participantName, msg.requestTarget.participantName);
        return;
    }

    if (_participantId == REGISTRY_PARTICIPANT_ID)
    {
        ReceiveRemoteParticipantConnectRequest_Registry(peer, std::move(msg));
        return;
    }
    else
    {
        ReceiveRemoteParticipantConnectRequest_Participant(peer, std::move(msg));
        return;
    }
}

void VAsioConnection::ReceiveRemoteParticipantConnectRequest_Registry(IVAsioPeer* peer,
                                                                      RemoteParticipantConnectRequest msg)
{
    SILKIT_TRACE_METHOD_(_logger, "({} {} {})", msg.requestOrigin.participantName, msg.requestTarget.participantName,
                         static_cast<int>(msg.status));

    if (msg.status == RemoteParticipantConnectRequest::ANNOUNCEMENT)
    {
        SilKit::Services::Logging::Error(_logger,
                                         "Ignoring RemoteParticipantConnectRequest announcement (origin={}, target={})",
                                         msg.requestOrigin.participantName, msg.requestTarget.participantName);
        return;
    }

    IVAsioPeer* destination{nullptr};

    switch (msg.status)
    {
    case RemoteParticipantConnectRequest::REQUEST:
        destination = FindPeerByName(peer->GetSimulationName(), msg.requestTarget.participantName);

        if (destination != nullptr)
        {
            // update the acceptor peer info of the requests origin (particularly the acceptor URIs)
            msg.requestOrigin = peer->GetInfo();
            msg.requestOrigin.acceptorUris = TransformAcceptorUris(_logger, peer, destination);
        }

        break;
    case RemoteParticipantConnectRequest::CONNECTING:
    case RemoteParticipantConnectRequest::FAILED_TO_CONNECT:
        destination = FindPeerByName(peer->GetSimulationName(), msg.requestOrigin.participantName);
        break;

    default:
        // The registry only cares about REQUEST, CONNECTING, and CONNECTING_FAILED
        break;
    }

    if (destination == nullptr)
    {
        SilKit::Services::Logging::Error(
            _logger, "Ignoring invalid RemoteParticipantConnectRequest (origin={}, target={}, status={})",
            msg.requestOrigin.participantName, msg.requestTarget.participantName, static_cast<int>(msg.status));
        return;
    }

    SerializedMessage buffer{msg};
    destination->SendSilKitMsg(buffer);
}

void VAsioConnection::ReceiveRemoteParticipantConnectRequest_Participant(IVAsioPeer* peer,
                                                                         RemoteParticipantConnectRequest msg)
{
    SILKIT_TRACE_METHOD_(_logger, "({} {} {})", msg.requestOrigin.participantName, msg.requestTarget.participantName,
                         static_cast<int>(msg.status));

    const bool isOrigin{msg.requestOrigin.participantName == _participantName};
    const bool isTarget{msg.requestTarget.participantName == _participantName};

    if (!isOrigin && isTarget)
    {
        if (msg.status == RemoteParticipantConnectRequest::REQUEST)
        {
            SilKit::Services::Logging::Debug(
                _logger, "Received RemoteParticipantConnectRequest::REQUEST (request origin {}, request target {})",
                msg.requestOrigin.participantName, msg.requestTarget.participantName);

            // XXX check if already connected to origin

            _remoteConnectionManager.StartConnectingTo(msg.requestOrigin);

            // Notify the remote peer that we are now attempting to connect. The remote peer will then wait for the
            // incoming connection, or the FAILED_TO_CONNECT status message, which will initiate the connection via the
            // registry proxy.
            msg.status = RemoteParticipantConnectRequest::CONNECTING;
            SerializedMessage buffer{msg};
            peer->SendSilKitMsg(std::move(buffer));

            return;
        }
    }

    if (isOrigin && !isTarget)
    {
        if (msg.status == RemoteParticipantConnectRequest::ANNOUNCEMENT)
        {
            SilKit::Services::Logging::Debug(
                _logger,
                "Received RemoteParticipantConnectRequest::ANNOUNCEMENT (request origin {}, request target {})",
                msg.requestOrigin.participantName, msg.requestTarget.participantName);

            // The remote participant informs us that this is a remote-connection and we should start the
            // participant-participant handshake (i.e., send the ParticipantAnnouncement).
            peer->SetInfo(msg.requestTarget);

            _connectKnownParticipants.HandlePeerEvent(
                msg.requestTarget.participantName,
                ConnectKnownParticipants::PeerEvent::REMOTE_PARTICIPANT_ANNOUNCEMENT);

            HandleConnectedPeer(peer);
            return;
        }

        if (msg.status == RemoteParticipantConnectRequest::CONNECTING)
        {
            SilKit::Services::Logging::Debug(
                _logger, "Received RemoteParticipantConnectRequest::CONNECTING (request origin {}, request target {})",
                msg.requestOrigin.participantName, msg.requestTarget.participantName);

            // The remote participant informs us that it received the REQUEST and is starting to connect to us.
            _connectKnownParticipants.HandlePeerEvent(
                msg.requestTarget.participantName,
                ConnectKnownParticipants::PeerEvent::REMOTE_PARTICIPANT_IS_CONNECTING);

            return;
        }

        if (msg.status == RemoteParticipantConnectRequest::FAILED_TO_CONNECT)
        {
            SilKit::Services::Logging::Debug(
                _logger,
                "Received RemoteParticipantConnectRequest::FAILED_TO_CONNECT (request origin {}, request target {})",
                msg.requestOrigin.participantName, msg.requestTarget.participantName);

            _connectKnownParticipants.HandlePeerEvent(
                msg.requestTarget.participantName,
                ConnectKnownParticipants::PeerEvent::REMOTE_PARTICIPANT_FAILED_TO_CONNECT);

            return;
        }
    }

    SilKit::Services::Logging::Error(
        _logger, "Ignoring invalid RemoteParticipantConnectRequest (origin={}, target={}, status={})",
        msg.requestOrigin.participantName, msg.requestTarget.participantName, static_cast<int>(msg.status));
}


void VAsioConnection::HandleConnectedPeer(IVAsioPeer* peer)
{
    const auto& peerInfo{peer->GetInfo()};

    // Since we are connecting to the peer, we already know which simulation name we expect.
    peer->SetSimulationName(_simulationName);

    // We connected to the other peer. tell him who we are.
    SendParticipantAnnouncement(peer);

    // The service ID is incomplete at this stage.
    ServiceDescriptor peerId;
    peerId.SetParticipantNameAndComputeId(peerInfo.participantName);
    peer->SetServiceDescriptor(peerId);

    AssociateParticipantNameAndPeer(_simulationName, peer->GetInfo().participantName, peer);
}


void VAsioConnection::AssociateParticipantNameAndPeer(const std::string& simulationName,
                                                      const std::string& participantName, IVAsioPeer* peer)
{
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        _participantNameToPeer[simulationName].insert({participantName, peer});
    }

    IStringListMetric* metric;
    auto metricNameBase = "Peer/" + simulationName + "/" + participantName;

    metric = _metricsManager->GetStringList(metricNameBase + "/LocalEndpoint");
    metric->Add(peer->GetLocalAddress());

    metric = _metricsManager->GetStringList(metricNameBase + "/RemoteEndpoint");
    metric->Add(peer->GetRemoteAddress());
}

auto VAsioConnection::FindPeerByName(const std::string& simulationName,
                                     const std::string& participantName) const -> IVAsioPeer*
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    auto simulationIt{_participantNameToPeer.find(simulationName)};
    if (simulationIt == _participantNameToPeer.end())
    {
        return nullptr;
    }

    auto participantIt{simulationIt->second.find(participantName)};
    if (participantIt == simulationIt->second.end())
    {
        return nullptr;
    }

    return participantIt->second;
}


void VAsioConnection::StartIoWorker()
{
    // do nothing if the worker thread is already running
    if (_ioWorker.get_id() != std::thread::id{})
    {
        return;
    }

    _ioWorker = std::thread{[this]() {
        SilKit::Util::SetThreadName(("IO " + _participantName).substr(0, 15));

        while (true)
        {
            try
            {
                _ioContext->Run();
                return;
            }
            catch (const std::exception& error)
            {
                Services::Logging::Error(_logger, "SilKit-IOWorker: Something went wrong: {}", error.what());
            }
        }
    }};
}

void VAsioConnection::AcceptLocalConnections(const std::string& uniqueId)
{
    auto localEndpoint = makeLocalEndpoint(_participantName, _participantId, uniqueId);

    // file must not exist before we bind/listen on it
    (void)fs::remove(localEndpoint.path());

    try
    {
        auto acceptor{_ioContext->MakeLocalAcceptor(localEndpoint.path())};
        acceptor->SetListener(*this);
        acceptor->AsyncAccept({});

        Services::Logging::Debug(_logger, "SIL Kit is listening on {}", acceptor->GetLocalEndpoint());

        {
            std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};
            _acceptors.emplace_back(std::move(acceptor));
        }
    }
    catch (const std::exception& exception)
    {
        Services::Logging::Error(_logger, "SIL Kit failed to listening on {}: {}", localEndpoint.path(),
                                 exception.what());
        throw;
    }
}

auto VAsioConnection::AcceptTcpConnectionsOn(const std::string& hostName,
                                             uint16_t port) -> std::pair<std::string, uint16_t>
{
    // Default to TCP IPv4 catchallIp
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);

    if (!hostName.empty())
    {
        auto resolverResults{_ioContext->Resolve(hostName)};

        if (resolverResults.empty())
        {
            Services::Logging::Error(_logger, "AcceptTcpConnectionsOn: Unable to resolve hostname\"{}:{}\"", hostName,
                                     port);
            throw SilKit::StateError{"Unable to resolve hostname and service."};
        }

        std::vector<asio::ip::tcp::endpoint> endpoints;
        for (const auto& address : resolverResults)
        {
            endpoints.emplace_back(asio::ip::make_address(address), port);
        }

        endpoint = selectBestEndpointFromResolverResults(endpoints);

        Services::Logging::Debug(_logger, "Accepting connections at {}:{} @{}", endpoint.address().to_string(),
                                 endpoint.port(), (endpoint.address().is_v4() ? "TCPv4" : "TCPv6"));
    }

    try
    {
        auto acceptor{_ioContext->MakeTcpAcceptor(endpoint.address().to_string(), port)};
        acceptor->SetListener(*this);
        acceptor->AsyncAccept({});

        auto localEndpointUri{Uri::Parse(acceptor->GetLocalEndpoint())};

        Services::Logging::Debug(_logger, "SIL Kit is listening on {}", localEndpointUri.EncodedString());

        {
            std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};
            _acceptors.emplace_back(std::move(acceptor));
        }

        return std::make_pair(localEndpointUri.Host(), localEndpointUri.Port());
    }
    catch (const std::exception& exception)
    {
        Services::Logging::Error(_logger, "SIL Kit failed to listening on {}:{}: {}", endpoint.address().to_string(),
                                 endpoint.port(), exception.what());
        throw;
    }
}

void VAsioConnection::AddPeer(std::unique_ptr<IVAsioPeer> newPeer)
{
    newPeer->StartAsyncRead();

    if (_useAggregation)
    {
        newPeer->EnableAggregation();
    }

    std::unique_lock<std::mutex> lock{_peersLock};

    auto* const proxyPeer = dynamic_cast<VAsioProxyPeer*>(newPeer.get());
    if (proxyPeer != nullptr)
    {
        _peerToProxyPeers[proxyPeer->GetPeer()].insert(proxyPeer);
    }

    _peers.emplace_back(std::move(newPeer));
}

void VAsioConnection::RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> callback)
{
    ExecuteOnIoThread(
        [this, callback{std::move(callback)}] { _peerShutdownCallbacks.emplace_back(std::move(callback)); });
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
    const auto& sourceSimulationName = peer->GetSimulationName();
    const auto& sourceParticipantName = peer->GetInfo().participantName;

    const auto simulationIt = _proxySourceToDestinations.find(sourceSimulationName);
    if (simulationIt != _proxySourceToDestinations.end())
    {
        const auto proxyDestinationsIt = simulationIt->second.find(sourceParticipantName);
        if (proxyDestinationsIt != simulationIt->second.end())
        {
            for (const auto& destination : proxyDestinationsIt->second)
            {
                auto destinationPeer{FindPeerByName(sourceSimulationName, destination)};

                // if a destination participant name has no associated peer, ignore it, it was already been disconnected
                if (destinationPeer == nullptr)
                {
                    continue;
                }

                ProxyMessage msg{};
                msg.source = sourceParticipantName;
                msg.destination = destination;
                msg.payload.clear();

                destinationPeer->SendSilKitMsg(SerializedMessage{msg});
            }
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
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};

        auto simulationIt{_participantNameToPeer.find(peer->GetSimulationName())};
        if (simulationIt != _participantNameToPeer.end())
        {
            simulationIt->second.erase(peer->GetInfo().participantName);
        }
    }

    auto it{
        std::find_if(_peers.begin(), _peers.end(), [needle = peer](const auto& hay) { return hay.get() == needle; })};

    if (it != _peers.end())
    {
        _peers.erase(it);
    }
}

void VAsioConnection::NotifyShutdown()
{
    _isShuttingDown = true;
}

void VAsioConnection::EnableAggregation()
{
    // pass information to all existing peers
    for (auto& peer : _peers)
    {
        peer->EnableAggregation();
    }

    // keep information for peers joining in the future
    _useAggregation = true;
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

    if (!_capabilities.HasProxyMessageCapability())
    {
        static SilKit::Services::Logging::LogOnceFlag onceFlag;
        SilKit::Services::Logging::Warn(
            _logger, onceFlag,
            "Ignoring VAsioMsgKind::SilKitProxyMessage because feature is disabled via configuration: From {}, To {}",
            proxyMessage.source, proxyMessage.destination);
        return;
    }

    const auto& fromSimulationName{from->GetSimulationName()};

    SilKit::Services::Logging::Trace(_logger,
                                     "Received message with VAsioMsgKind::SilKitProxyMessage: From {} ({}), To {}",
                                     proxyMessage.source, fromSimulationName, proxyMessage.destination);


    const bool fromIsSource = from->GetInfo().participantName == proxyMessage.source;
    if (fromIsSource)
    {
        auto peer{FindPeerByName(fromSimulationName, proxyMessage.destination)};
        if (peer == nullptr)
        {
            SilKit::Services::Logging::Error(_logger, "Unable to deliver proxy message from {} to {} in simulation {}",
                                             proxyMessage.source, proxyMessage.destination, fromSimulationName);
            return;
        }

        peer->SendSilKitMsg(SerializedMessage{proxyMessage});

        // We are relaying a message from source to destination and acting as a proxy. Record the association between
        // source and destination. This is used during disconnects, where we create empty ProxyMessages on behalf of
        // the disconnected peer, to inform the destination that the source peer has disconnected.
        _proxySourceToDestinations[fromSimulationName][proxyMessage.source].insert(proxyMessage.destination);

        return;
    }

    const bool isDestination = _participantName == proxyMessage.destination;
    if (isDestination)
    {
        auto peer{FindPeerByName(_simulationName, proxyMessage.source)};

        if (peer == nullptr)
        {
            SilKit::Services::Logging::Debug(_logger, "Creating VAsioProxyPeer ({})", proxyMessage.source);

            auto proxyPeer = std::make_unique<VAsioProxyPeer>(this, _participantName, VAsioPeerInfo{}, from, _logger);
            peer = proxyPeer.get();

            AddPeer(std::move(proxyPeer));
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

        tt::for_each(supportedMessageTypes, [&subscriptionVersion, &typeName, remoteVersion](auto&& myType) {
            using MsgT = std::decay_t<decltype(myType)>;
            if (typeName == SilKitMsgTraits<MsgT>::SerdesName())
            {
                if (SilKitMsgTraits<MsgT>::Version() <= remoteVersion)
                {
                    subscriptionVersion = SilKitMsgTraits<MsgT>::Version();
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
    ack.status = wasAdded ? SubscriptionAcknowledge::Status::Success : SubscriptionAcknowledge::Status::Failed;

    from->SendSilKitMsg(SerializedMessage{from->GetProtocolVersion(), ack});
}

void VAsioConnection::ReceiveSubscriptionAcknowledge(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto ack = buffer.Deserialize<SubscriptionAcknowledge>();

    if (ack.status != SubscriptionAcknowledge::Status::Success)
    {
        Services::Logging::Error(_logger, "Failed to subscribe [{}] {} from {}", ack.subscriber.networkName,
                                 ack.subscriber.msgTypeName, from->GetInfo().participantName);
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
                                 subscriber.msgTypeName, subscriber.networkName, from->GetInfo().participantName);
    }
    else
    {
        Services::Logging::Warn(
            _logger, "Participant '{}' could not be registered as receiver for messages of type '{}' on link '{}'",
            from->GetInfo().participantName, subscriber.msgTypeName, subscriber.networkName);
    }

    return wasAdded;
}

void VAsioConnection::ReceiveRawSilKitMessage(IVAsioPeer* from, SerializedMessage&& buffer)
{
    auto receiverIdx = static_cast<size_t>(buffer.GetRemoteIndex()); //ExtractEndpointId(buffer);
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

    const auto PreambleIsPresent = [](const ProtocolVersion& version, const SerializedMessage& buffer) -> bool {
        const auto kind{buffer.GetRegistryKind()};

        if (version == ProtocolVersion{3, 0})
        {
            if (kind == RegistryMessageKind::ParticipantAnnouncementReply)
            {
                return false;
            }
        }

        if (version == ProtocolVersion{3, 1})
        {
            if (kind == RegistryMessageKind::RemoteParticipantConnectRequest)
            {
                return false;
            }
        }

        return true;
    };

    if (PreambleIsPresent(from->GetProtocolVersion(), buffer)
        && header.preamble != REGISTRY_MESSAGE_HEADER_PREAMBLE_VALUE)
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
        return ReceiveRemoteParticipantConnectRequest(from, std::move(buffer));
    }
}

void VAsioConnection::AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    if (_hasPendingAsyncSubscriptions)
    {
        _asyncSubscriptionsCompletionHandlers.Add(std::move(handler));
    }
    else
    {
        handler();
    }
}

auto VAsioConnection::GetNumberOfRemoteReceivers(const IServiceEndpoint* service,
                                                 const std::string& msgTypeName) -> size_t
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
                                                           const std::string& msgTypeName) -> std::vector<std::string>
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
    _asyncSubscriptionsCompletionHandlers.InvokeAll();
    _asyncSubscriptionsCompletionHandlers.Clear();
    _hasPendingAsyncSubscriptions = false;
}

bool VAsioConnection::ParticipantHasCapability(const std::string& participantName, const std::string& capability) const
{
    const auto peer{FindPeerByName(_simulationName, participantName)};
    if (peer == nullptr)
    {
        SilKit::Services::Logging::Warn(_logger, "ParticipantHasCapability: Participant '{}' unknown", participantName);
        return false;
    }

    auto capabilities = SilKit::Core::VAsioCapabilities{peer->GetInfo().capabilities};
    return capabilities.HasCapability(capability);
}


auto VAsioConnection::GetIoContext() -> IIoContext*
{
    return _ioContext.get();
}


auto VAsioConnection::MakePeerInfo() -> VAsioPeerInfo
{
    VAsioPeerInfo peerInfo;

    peerInfo.participantName = _participantName;
    peerInfo.participantId = _participantId;

    {
        std::lock_guard<decltype(_acceptorsMutex)> lock{_acceptorsMutex};

        // Ensure that the local acceptors are the first entries in the acceptorUris
        for (const auto& acceptor : _acceptors)
        {
            Uri uri{acceptor->GetLocalEndpoint()};

            if (uri.Type() != Uri::UriType::Local)
            {
                continue;
            }

            peerInfo.acceptorUris.emplace_back(uri.EncodedString());
        }

        for (const auto& acceptor : _acceptors)
        {
            Uri uri{acceptor->GetLocalEndpoint()};

            if (uri.Type() != Uri::UriType::Tcp)
            {
                continue;
            }

            peerInfo.acceptorUris.emplace_back(uri.EncodedString());
        }
    }

    peerInfo.capabilities = MakeCapabilitiesStringFromConfiguration(_config);

    return peerInfo;
}


auto VAsioConnection::MakeConnectPeer(const VAsioPeerInfo& peerInfo) -> std::unique_ptr<IConnectPeer>
{
    auto connectPeer{
        std::make_unique<ConnectPeer>(_ioContext.get(), _logger, peerInfo, _config.middleware.enableDomainSockets)};
    return connectPeer;
}


auto VAsioConnection::MakeVAsioPeer(std::unique_ptr<IRawByteStream> stream) -> std::unique_ptr<IVAsioPeer>
{
    auto vAsioPeer{std::make_unique<VAsioPeer>(this, _ioContext.get(), std::move(stream), _logger)};
    return vAsioPeer;
}


// IAcceptorListener


void VAsioConnection::OnAsyncAcceptSuccess(IAcceptor& acceptor, std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&acceptor));

    Services::Logging::Debug(_logger, "New connection from [local={}, remote={}]", stream->GetLocalEndpoint(),
                             stream->GetRemoteEndpoint());

    try
    {
        auto vAsioPeer{MakeVAsioPeer(std::move(stream))};
        AddPeer(std::move(vAsioPeer));
    }
    catch (const std::exception& exception)
    {
        Services::Logging::Error(_logger, "SIL Kit cannot create listener socket: {}", exception.what());
        throw;
    }

    acceptor.AsyncAccept({});
}


void VAsioConnection::OnAsyncAcceptFailure(IAcceptor& acceptor)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&acceptor));

    {
        std::unique_lock<decltype(_acceptorsMutex)> lock{_acceptorsMutex};

        auto it{std::find_if(_acceptors.begin(), _acceptors.end(),
                             [needle = &acceptor](const auto& hay) { return hay.get() == needle; })};

        if (it != _acceptors.end())
        {
            _acceptors.erase(it);
        }
    }
}


// IConnectionManagerListener


void VAsioConnection::OnConnectKnownParticipantsFailure(ConnectKnownParticipants& connectKnownParticipants)
{
    auto message{fmt::format("Failed to connect to known participants: {}", connectKnownParticipants.Describe())};
    _logger->Error(message);

    try
    {
        _startWaitingForParticipantHandshakes.set_exception(std::make_exception_ptr(SilKit::ProtocolError{message}));
    }
    catch (...)
    {
    }

    try
    {
        _allKnownParticipantHandshakesComplete.set_exception(std::make_exception_ptr(SilKit::ProtocolError{message}));
    }
    catch (...)
    {
    }
}

void VAsioConnection::OnConnectKnownParticipantsWaitingForAllReplies(ConnectKnownParticipants&)
{
    Log::Debug(_logger, "Waiting for completion of all handshakes with all known participants");

    try
    {
        _startWaitingForParticipantHandshakes.set_value();
    }
    catch (...)
    {
    }
}

void VAsioConnection::OnConnectKnownParticipantsAllRepliesReceived(ConnectKnownParticipants&)
{
    Log::Debug(_logger, "All handshakes with all known participants are complete");

    try
    {
        _allKnownParticipantHandshakesComplete.set_value();
    }
    catch (...)
    {
    }
}


void VAsioConnection::OnRemoteConnectionSuccess(std::unique_ptr<SilKit::Core::IVAsioPeer> vAsioPeer)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", vAsioPeer->GetInfo().participantName);

    RemoteParticipantConnectRequest msg;
    msg.messageHeader = MakeRegistryMsgHeader(_version);
    msg.requestOrigin = vAsioPeer->GetInfo();
    msg.requestTarget = MakePeerInfo();
    msg.status = RemoteParticipantConnectRequest::ANNOUNCEMENT;

    SerializedMessage buffer{msg};
    vAsioPeer->SendSilKitMsg(std::move(buffer));

    AddPeer(std::move(vAsioPeer));
}

void VAsioConnection::OnRemoteConnectionFailure(SilKit::Core::VAsioPeerInfo peerInfo)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", peerInfo.participantName);

    RemoteParticipantConnectRequest msg;
    msg.messageHeader = MakeRegistryMsgHeader(_version);
    msg.requestOrigin = std::move(peerInfo);
    msg.requestTarget = MakePeerInfo();
    msg.status = RemoteParticipantConnectRequest::FAILED_TO_CONNECT;

    SerializedMessage buffer{msg};
    _registry->SendSilKitMsg(std::move(buffer));
}


bool VAsioConnection::TryRemoteConnectRequest(VAsioPeerInfo const& peerInfo)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", peerInfo.participantName);

    SilKit::Services::Logging::Debug(_logger, "Trying to request remote connection from {} via the registry",
                                     peerInfo.participantName);

    if (!_capabilities.HasCapability(Capabilities::RequestParticipantConnection))
    {
        SilKit::Services::Logging::Warn(
            _logger, "Cannot request remote connection from {}, because it is disabled in the configuration",
            peerInfo.participantName);

        return false;
    }

    try
    {
        const VAsioCapabilities peerCapabilities{peerInfo.capabilities};
        if (!peerCapabilities.HasCapability(Capabilities::RequestParticipantConnection))
        {
            SilKit::Services::Logging::Warn(_logger,
                                            "Cannot request remote connection from {}, because {} does not support it",
                                            peerInfo.participantName, peerInfo.participantName);

            return false;
        }
    }
    catch (const std::exception& error)
    {
        SilKit::Services::Logging::Error(_logger, "Failed to parse capabilities string: {}", error.what());
        return false;
    }
    catch (...)
    {
        SilKit::Services::Logging::Error(_logger, "Failed to parse capabilities string: unknown error");
        return false;
    }

    RemoteParticipantConnectRequest request;
    request.messageHeader = MakeRegistryMsgHeader(_version);
    request.requestOrigin = MakePeerInfo();
    request.requestTarget = peerInfo;
    request.status = RemoteParticipantConnectRequest::REQUEST;

    SerializedMessage buffer{request};
    _registry->SendSilKitMsg(std::move(buffer));

    return true;
}


bool VAsioConnection::TryProxyConnect(VAsioPeerInfo const& peerInfo)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", peerInfo.participantName);

    SilKit::Services::Logging::Debug(_logger, "Trying to use the registry as a proxy to communicate with {}",
                                     peerInfo.participantName);

    // NB: Cannot check the capabilities of the registry, since we do not receive the PeerInfo from the
    //       registry over the network, but build it ourselves in VAsioConnection::JoinSimulation.
    //       This is not be a huge issue, since we can just 'throw the messages at the registry' and will
    //       fail with the participant-connection-timeout if it is not capable of routing it to the other
    //       participant.

    if (!_capabilities.HasProxyMessageCapability())
    {
        SilKit::Services::Logging::Warn(
            _logger,
            "Cannot use the registry as a proxy to communicate with {}, because it is disabled in the configuration",
            peerInfo.participantName);

        return false;
    }

    const VAsioCapabilities peerCapabilities{peerInfo.capabilities};
    if (!peerCapabilities.HasCapability(Capabilities::ProxyMessage))
    {
        SilKit::Services::Logging::Warn(_logger,
                                        "VAsioConnection: Cannot use the registry as a proxy to communicate with {}, "
                                        "because {} does not support it",
                                        peerInfo.participantName, peerInfo.participantName);

        return false;
    }

    auto peer{std::make_unique<VAsioProxyPeer>(this, _participantName, peerInfo, _registry.get(), _logger)};
    HandleConnectedPeer(peer.get());
    AddPeer(std::move(peer));

    return true;
}


} // namespace Core
} // namespace SilKit


#undef SILKIT_TRACE_METHOD_
