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

using asio::ip::tcp;

namespace SilKit {
namespace Core {

VAsioRegistry::VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg, ProtocolVersion version) :
    _vasioConfig{ std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(cfg) },
    _connection{ nullptr,  *_vasioConfig, "SilKitRegistry", VAsioConnection::RegistryParticipantId, &_timeProvider, version}
{
    _logger = std::make_unique<Services::Logging::Logger>("SilKitRegistry", _vasioConfig->logging);
    _connection.SetLogger(_logger.get());

    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement)
    {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { OnPeerShutdown(peer); });
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
        uri = Uri{tcpHostPort.first, tcpHostPort.second};

        hasTcpSocket = true;
    }
    catch (const std::exception& e)
    {
        Services::Logging::Error(GetLogger(), "SIL Kit Registry failed to create listening socket {}:{} (uri: {}). Reason: {}",
                       uri.Host(),
                       uri.Port(),
                       uri.EncodedString(),
                       e.what());
    }

    if (enableDomainSockets)
    {
        try
        {
            _connection.AcceptLocalConnections(listenUri);

            hasDomainSocket = true;
        }
        catch (const std::exception& e)
        {
            Services::Logging::Warn(GetLogger(), "SIL Kit Registry failed to create local listening socket: {}", e.what());
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

    _connection.StartIoWorker();

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

auto VAsioRegistry::FindConnectedPeer(const std::string& name) const -> std::vector<ConnectedParticipantInfo>::const_iterator
{
    return std::find_if(_connectedParticipants.begin(), _connectedParticipants.end(),
        [&name](const auto& connectedParticipant) { return connectedParticipant.peerInfo.participantName == name; });
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement)
{
    const auto& peerInfo = announcement.peerInfo;

    // NB When we have a remote client we might need to patch its acceptor name (host or ip address).
    // In a distributed simulation the participants will listen on an IPADDRANY address
    // without explicitly specifying on which network interface they are listening.
    // When the IVAsioPeer connects to us we see its actual endpoint address and need
    // to substitute it here.

    // Do not allow multiple participants with identical names
    if(FindConnectedPeer(peerInfo.participantName) != _connectedParticipants.end())
    {
        Services::Logging::Warn(
                GetLogger(),
                "A participant with the same name '{}' already exists", peerInfo.participantName);
        throw SilKitError{fmt::format("A Participant with the name '{}' already exists!", peerInfo.participantName)};
    }

    SendKnownParticipants(from);

    ConnectedParticipantInfo newParticipantInfo;
    newParticipantInfo.peer = from;
    newParticipantInfo.peerInfo = peerInfo;
    _connectedParticipants.emplace_back(std::move(newParticipantInfo));

    if (AllParticipantsAreConnected())
    {
        _logger->Info("All participants are online");
        if (_onAllParticipantsConnected)
            _onAllParticipantsConnected();
    }
}

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer)
{
    Services::Logging::Info(GetLogger(), "Sending known participant message to {}", peer->GetInfo().participantName);

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());

    for (const auto& connectedParticipant : _connectedParticipants)
    {
        // don't advertise the peer to itself
        if (connectedParticipant.peer == peer) continue;

        auto peerInfo = connectedParticipant.peerInfo;
        peerInfo.acceptorUris = TransformAcceptorUris(GetLogger(), connectedParticipant.peer, peer);

        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(std::remove_if(_connectedParticipants.begin(), _connectedParticipants.end(),
        [peer](const auto& connectedParticipant) {
            return connectedParticipant.peer == peer;
        })
        , _connectedParticipants.end()
    );

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

} // namespace Core
} // namespace SilKit

