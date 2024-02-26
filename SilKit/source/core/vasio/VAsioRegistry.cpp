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

#include "VAsioRegistry.hpp"

#include "Logger.hpp"
#include "Uri.hpp"
#include "ILogger.hpp"
#include "Optional.hpp"
#include "TransformAcceptorUris.hpp"
#include "VAsioConstants.hpp"


namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {


VAsioRegistry::VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg, ProtocolVersion version)
    : VAsioRegistry(std::move(cfg), nullptr, version)
{
}

VAsioRegistry::VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                             IRegistryEventListener* registryEventListener, ProtocolVersion version)
    : _registryEventListener{registryEventListener}
    , _vasioConfig{std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(cfg)}
    , _connection{nullptr, *_vasioConfig, REGISTRY_PARTICIPANT_NAME, REGISTRY_PARTICIPANT_ID, &_timeProvider, version}
{
    _logger = std::make_unique<Services::Logging::Logger>(REGISTRY_PARTICIPANT_NAME, _vasioConfig->logging);

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnLoggerCreated(_logger.get());
    }

    _connection.SetLogger(_logger.get());

    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement) {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) {
        OnPeerShutdown(peer);
    });

    _serviceDescriptor.SetParticipantNameAndComputeId(REGISTRY_PARTICIPANT_NAME);
    _serviceDescriptor.SetParticipantId(REGISTRY_PARTICIPANT_ID);
    _serviceDescriptor.SetNetworkName("default");
}

auto VAsioRegistry::StartListening(const std::string& listenUri) -> std::string
{
    auto uri = Uri::Parse(listenUri);
    const auto enableDomainSockets = _vasioConfig->middleware.enableDomainSockets;

    bool hasTcpSocket = false;
    bool hasDomainSocket = false;

    try
    {
        // Resolve the configured hostname and accept on the given port:
        auto tcpHostPort = _connection.AcceptTcpConnectionsOn(uri.Host(), uri.Port());
        // Update the URI to the actually used address and port:
        uri = Uri{uri.Host(), tcpHostPort.second};

        hasTcpSocket = true;
    }
    catch (const std::exception& e)
    {
        Services::Logging::Error(GetLogger(),
                                 "SIL Kit Registry failed to create listening socket {}:{} (uri: {}). Reason: {}",
                                 uri.Host(), uri.Port(), uri.EncodedString(), e.what());
    }

    if (enableDomainSockets)
    {
        try
        {
            // If we have opened a TCP socket, use the URI we're going to return to the caller. Otherwise, use the URI
            // passed as the argument.
            std::string uniqueId{listenUri};
            if (hasTcpSocket)
            {
                uniqueId = uri.EncodedString();
            }

            _connection.AcceptLocalConnections(uniqueId);

            hasDomainSocket = true;
        }
        catch (const std::exception& e)
        {
            Services::Logging::Warn(GetLogger(), "SIL Kit Registry failed to create local listening socket: {}",
                                    e.what());
        }
    }

    if (hasTcpSocket && hasDomainSocket)
    {
        Services::Logging::Debug(GetLogger(), "SIL Kit Registry: Listening on both, TCP and Domain sockets");
    }
    else if (hasTcpSocket && !hasDomainSocket)
    {
        if (enableDomainSockets)
        {
            // There exist old versions of Windows that do not support domain sockets. Here only TCP/IP will be available.
            _logger->Warn(
                "This registry instance will only accept connections on TCP sockets. This might be caused by a second "
                "registry running on this host, or using an operating system that does not support Domain sockets.");
        }
        else
        {
            _logger->Warn("This registry instance will only accept connections on TCP sockets. Domain sockets were "
                          "explicitly disabled through the participant configuration.");
        }
    }
    else if (!hasTcpSocket && hasDomainSocket)
    {
        // For scenarios where multiple instances run on the same host, binding on TCP/IP will result in an error.
        // However, if we can accept local ipc connections we warn and continue.
        _logger->Warn("This registry instance will only accept connections on local domain sockets. This might be "
                      "caused by a second registry running on this host, or another process was already bound to the "
                      "same port as this registry was attempting to use.");
    }
    else
    {
        Services::Logging::Error(GetLogger(), "SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets");
        throw SilKit::StateError{"SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets"};
    }

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnRegistryUri(uri.EncodedString());
    }

    _connection.StartIoWorker();

    _connection.RegisterSilKitService(this);

    return uri.EncodedString();
}

void VAsioRegistry::SetAllConnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsConnected = std::move(handler);
}
void VAsioRegistry::SetAllDisconnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsDisconnected = std::move(handler);
}
auto VAsioRegistry::GetLogger() -> Services::Logging::ILogger*
{
    return _logger.get();
}

auto VAsioRegistry::FindConnectedParticipant(const std::string& participantName,
                                              const std::string& simulationName) const
    -> const ConnectedParticipantInfo*
{
    const auto simulationIt{_connectedParticipants.find(simulationName)};
    if (simulationIt == _connectedParticipants.end())
    {
        return nullptr;
    }

    const auto& simulationParticipants{simulationIt->second};

    const auto participantIt{simulationParticipants.find(participantName)};
    if (participantIt == simulationParticipants.end())
    {
        return nullptr;
    }

    return std::addressof(participantIt->second);
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement)
{
    const auto& peerInfo = announcement.peerInfo;

    // NB When we have a remote client we might need to patch its acceptor name (host or ip address).
    // In a distributed simulation the participants will listen on an IPADDRANY address
    // without explicitly specifying on which network interface they are listening.
    // When the IVAsioPeer connects to us we see its actual endpoint address and need
    // to substitute it here.

    // Do not allow multiple participants with identical names
    if (FindConnectedParticipant(peerInfo.participantName, announcement.simulationName) != nullptr)
    {
        const auto message = fmt::format("A participant with the same name '{}' already exists in the simulation {}",
                                         peerInfo.participantName, announcement.simulationName);
        GetLogger()->Warn(message);
        throw SilKitError{message};
    }

    SendKnownParticipants(peer, announcement.simulationName);

    ConnectedParticipantInfo participantInfo;
    participantInfo.peer = peer;
    participantInfo.peerInfo = peerInfo;
    _connectedParticipants[announcement.simulationName][peerInfo.participantName] = participantInfo;

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnParticipantConnected(announcement.simulationName, peerInfo.participantName);
    }

    if (AllParticipantsAreConnected())
    {
        _logger->Info("All participants are online");
        if (_onAllParticipantsConnected)
            _onAllParticipantsConnected();
    }
}

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName)
{
    Services::Logging::Info(GetLogger(), "Sending known participant message to {}, protocol version {}.{}",
                            peer->GetInfo().participantName, peer->GetProtocolVersion().major,
                            peer->GetProtocolVersion().minor);

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());

    const auto& simulationParticipants{_connectedParticipants[simulationName]};

    for (const auto& pPair : simulationParticipants)
    {
        const auto& connectedParticipant{pPair.second};

        // don't advertise the peer to itself
        if (connectedParticipant.peer == peer)
            continue;

        auto peerInfo = connectedParticipant.peerInfo;
        peerInfo.acceptorUris = TransformAcceptorUris(GetLogger(), connectedParticipant.peer, peer);

        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    namespace Log = SilKit::Services::Logging;

    const auto& simulationName{peer->GetSimulationName()};
    const auto& participantName{peer->GetInfo().participantName};

    if (FindConnectedParticipant(participantName, simulationName) == nullptr)
    {
        Log::Debug(_logger.get(), "Peer '{}' has shut down, which had no participant information", participantName);
        return;
    }

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnParticipantDisconnected(simulationName, participantName);
    }

    _connectedParticipants[simulationName].erase(participantName);

    if (_connectedParticipants[simulationName].empty())
    {
        _connectedParticipants.erase(simulationName);
    }

    if (_connectedParticipants.empty())
    {
        _logger->Info("All participants are shut down");
        if (_onAllParticipantsDisconnected)
            _onAllParticipantsDisconnected();
    }
}

bool VAsioRegistry::AllParticipantsAreConnected() const
{
    return false;
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Services::Orchestration::ParticipantStatus& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnParticipantStatusUpdate(serviceDescriptor.GetSimulationName(),
                                                      serviceDescriptor.GetParticipantName(), msg);
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Services::Orchestration::WorkflowConfiguration& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnRequiredParticipantsUpdate(
        serviceDescriptor.GetSimulationName(), serviceDescriptor.GetParticipantName(), msg.requiredParticipantNames);
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Core::Discovery::ServiceDiscoveryEvent& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnServiceDiscoveryEvent(serviceDescriptor.GetSimulationName(),
                                                    serviceDescriptor.GetParticipantName(), msg);
}

void VAsioRegistry::SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto VAsioRegistry::GetServiceDescriptor() const -> const ServiceDescriptor&
{
    return _serviceDescriptor;
}


} // namespace Core
} // namespace SilKit
