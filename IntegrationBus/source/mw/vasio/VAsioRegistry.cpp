// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioRegistry.hpp"

#include "Logger.hpp"
#include "Uri.hpp"

using asio::ip::tcp;

namespace {

bool isLocalhostAddress(const std::string& hostUrl)
{
    return hostUrl.find("tcp://127.") == 0 //ipv4
        || hostUrl.find("tcp://[::1]") == 0 //ipv6, abbreviated
        || hostUrl.find("tcp://[0:0:0:0:0:0:0:1]") == 0 //ipv6 addresses are long...
        ;
}

bool isCatchallAddress(const std::string& hostUrl)
{
    return hostUrl.find("tcp://0.0.0.0") == 0
        || hostUrl.find("tcp://[0:0:0:0:0:0:0:0]") == 0
        || hostUrl.find("tcp://[::]") == 0
        ;
}

} // namespace

namespace ib {
namespace mw {

VAsioRegistry::VAsioRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> cfg, ProtocolVersion version) :
    _vasioConfig{ std::dynamic_pointer_cast<ib::cfg::ParticipantConfiguration>(cfg) },
    _connection{ *_vasioConfig, "IbRegistry", VAsioConnection::RegistryParticipantId, version}
{
    _logger = std::make_unique<logging::Logger>("IbRegistry", _vasioConfig->logging);
    _connection.SetLogger(_logger.get());

    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement)
    {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { OnPeerShutdown(peer); });
}

void VAsioRegistry::ProvideDomain(const std::string& listenUri)
{
    auto uri = Uri::Parse(listenUri);
    bool isAccepting{false};
    // accept connection from participants on any interface
    try
    {
        //Local domain sockets, failure is non fatal for operation.
        _connection.AcceptLocalConnections(listenUri);
        isAccepting = true;
    }
    catch (const std::exception& e)
    {
        _logger->Warn("VAsioRegistry failed to create local listening socket: {}",
            e.what());
    }


    try
    {
        // Resolve the configured hostname and accept on the given port:
        _connection.AcceptTcpConnectionsOn(uri.Host(), uri.Port());
        isAccepting = true;
    }
    catch (const std::exception& e)
    {
        _logger->Error("VAsioRegistry failed to create listening socket {}:{} (uri: {}). Reason: {}",
            uri.Host(),
            uri.Port(),
            uri.EncodedString(),
            e.what());
        // For scenarios where multiple instances run on the same host, binding on TCP/IP
        // will result in an error. However, if we can accept local ipc connections we can
        // continue.
        if (!isAccepting)
        {
            throw;
        }
    }
    _connection.StartIoWorker();

}

void VAsioRegistry::SetAllConnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsConnected = std::move(handler);
}
void VAsioRegistry::SetAllDisconnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsDisconnected = std::move(handler);
}
auto VAsioRegistry::GetLogger() -> logging::ILogger*
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

    auto peerInfo = announcement.peerInfo;

    // NB When we have a remote client we might need to patch its acceptor name (host or ip address).
    // In a distributed simulation the participants will listen on an IPADDRANY address
    // without explicitly specifying on which network interface they are listening.
    // When the IVAsioPeer connects to us we see its actual endpoint address and need
    // to substitute it here.

    const auto fromUri = Uri{from->GetRemoteAddress()};
    if (fromUri.Type() == Uri::UriType::Tcp)
    {
        for (auto& uri : peerInfo.acceptorUris)
        {
            //parse to get listening port of peer
            const auto origUri = Uri{ uri };
            if (origUri.Type() == Uri::UriType::Local)
            {
                continue;
            }

            // Update to socket peer address but keep the original acceptor port.
            // Legacy clients have a hard-coded '127.0.0.1' address.
            if (isCatchallAddress(uri) || isLocalhostAddress(uri))
            {
                uri = Uri{ fromUri.Host(), origUri.Port() }.EncodedString();
            }
        }
    }

    if (FindConnectedPeer(peerInfo.participantName) != _connectedParticipants.end())
    {
        _logger->Warn(
            "Ignoring announcement from participant name={}, which is already connected",
            peerInfo.participantName);
        return;
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
    _logger->Info("Sending known participant message to {}", peer->GetInfo().participantName);

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = to_header(peer->GetProtocolVersion());
    // In case the peer is remote we need to replace all local addresses with 
    // the endpoint address known to the registry.
    auto replaceLocalhostUri = [&peer](auto& peerUriToPatch) {
        const auto registryUri = Uri{ peer->GetLocalAddress() };
        if (registryUri.Type() == Uri::UriType::Local)
        {
            // don't touch local domain socket URIs
            return;
        }
        for (auto& uri : peerUriToPatch.acceptorUris)
        {
            auto parsedUri = Uri{ uri };
            if (parsedUri.Type() == Uri::UriType::Local)
            {
                continue;
            }
            // Patch loopback or INADDR_ANY with the actual endpoint address of the remote peer's connection.
            if (isLocalhostAddress(uri) || isCatchallAddress(uri))
            {
                uri = Uri{ registryUri.Host(), parsedUri.Port() }.EncodedString();
            }
        }
    };

    for (const auto& connectedParticipant : _connectedParticipants)
    {
        // don't advertise the peer to itself
        if (connectedParticipant.peer == peer) continue;

        auto peerInfo = connectedParticipant.peerInfo;
        replaceLocalhostUri(peerInfo);
        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    peer->SendIbMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
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
    /*for (auto&& participant : _connection.Config().simulationSetup.participants)
    {
        if (FindConnectedPeer(participant.name) == _connectedParticipants.end())
        {
            return false;
        }
    }
    return true;*/
}

} // namespace mw
} // namespace ib

