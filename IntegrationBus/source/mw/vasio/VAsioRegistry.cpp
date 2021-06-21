// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioRegistry.hpp"

#include "ib/cfg/Config.hpp"
#include "Logger.hpp"

using asio::ip::tcp;

namespace ib {
namespace mw {

VAsioRegistry::VAsioRegistry(ib::cfg::Config cfg)
    : _vasioConfig{cfg.middlewareConfig.vasio}
    , _connection{cfg, "IbRegistry", 0}
{
    _logger = std::make_unique<logging::Logger>("IbRegistry", cfg.middlewareConfig.vasio.registry.logger);
    _connection.SetLogger(_logger.get());

    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement)
    {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { OnPeerShutdown(peer); });
}

void VAsioRegistry::ProvideDomain(uint32_t domainId)
{
    // accept connection from participants on any interface
    auto registryPort = static_cast<uint16_t>(_vasioConfig.registry.port + domainId);
    tcp::endpoint endpoint_v4(tcp::v4(), registryPort);
    try
    {
        //Local domain sockets
        _connection.AcceptLocalConnections();
        _connection.AcceptConnectionsOn(endpoint_v4);

        //tcp::endpoint endpoint_v6(tcp::v6(), registryPort);
        //FIXME allow ipv6: _connection.AcceptConnectionsOn(endpoint_v6);
    }
    catch (const std::exception& e)
    {
        _logger->Error("VAsioRegistry failed to create listening socket {} for domainId={}. Reason: {}",
            endpoint_v4,
            domainId,
            e.what());
        throw;
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

bool VAsioRegistry::IsExpectedParticipant(const ib::mw::VAsioPeerInfo& peerInfo)
{
    for (auto& participant : _connection.Config().simulationSetup.participants)
    {
        if (participant.id == peerInfo.participantId && participant.name == peerInfo.participantName)
        {
            return true;
        }
    }
    return false;
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* from, const ParticipantAnnouncement& announcement)
{
    auto& peerInfo = announcement.peerInfo;

    if (!IsExpectedParticipant(peerInfo))
    {
        _logger->Warn(
            "Ignoring announcement from unexpected participant name={} id={}",
            peerInfo.participantName,
            peerInfo.participantId);
        return;
    }


    SendKnownParticipants(from);

    _connectedParticipants[peerInfo.participantId] = peerInfo;

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

    for (auto&& connectedParticipant : _connectedParticipants)
    {
        auto&& peerInfo = connectedParticipant.second;
        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    MessageBuffer sendBuffer;
    uint32_t msgSizePlaceholder{0u};
    sendBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::IbRegistryMessage
        << RegistryMessageKind::KnownParticipants
        << knownParticipantsMsg;

    peer->SendIbMsg(std::move(sendBuffer));
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(peer->GetInfo().participantId);

    if (_connectedParticipants.empty())
    {
        _logger->Info("All participants are shut down");
        if (_onAllParticipantsDisconnected)
            _onAllParticipantsDisconnected();
    }
}

bool VAsioRegistry::AllParticipantsAreConnected() const
{
    for (auto&& participant : _connection.Config().simulationSetup.participants)
    {
        if (_connectedParticipants.count(participant.id) != 1)
        {
            return false;
        }
    }
    return true;
}

} // namespace mw
} // namespace ib

