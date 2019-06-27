// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Registry.hpp"

using namespace ib::mw;
using namespace ib::mw::registry;

using asio::ip::tcp;

Registry::Registry(ib::cfg::Config cfg)
{
    _config = cfg;
}

void Registry::ProvideDomain(uint32_t domainId)
{
    assert(!_tcpAcceptor);

    // accept connection from all participants
    // at the moment registry listens on 0.0.0.0:(42000+domainId)
    auto registryPort = static_cast<uint16_t>(42000 + domainId);
    _tcpAcceptor = std::make_unique<asio::ip::tcp::acceptor>(_ioContext, tcp::endpoint(tcp::v4(), registryPort));
    AcceptConnection();
    std::cout << "Listening on " << _tcpAcceptor->local_endpoint() << std::endl;

    StartIoWorker();

    _allParticipantsUp.get_future().wait();
    std::cout << "All Participants up" << std::endl;

    _allParticipantsDown.get_future().wait();
    std::cout << "All Participants down" << std::endl;
    std::cout << "Press enter to shutdown registry" << std::endl;
    std::cin.ignore();
}

auto Registry::ReceiveParticipantAnnoucement(MessageBuffer&& buffer, IVAsioPeer* peer) -> VAsioPeerInfo
{
    auto info = VAsioConnection::ReceiveParticipantAnnoucement(std::move(buffer), peer);

    registry::KnownParticipants knownParticipantsMsg;
    for (auto && connectedParticipant : _connectedParticipants)
    {
        knownParticipantsMsg.participantInfos.push_back(connectedParticipant.second);
    }

    _connectedParticipants[info.participantId] = info;

    MessageBuffer sendBuffer;
    uint32_t msgSizePlaceholder{ 0u };
    sendBuffer << msgSizePlaceholder
               << VAsioMsgKind::IbRegistryMessage
               << registry::RegistryMessageKind::KnownParticipants
               << knownParticipantsMsg;
    peer->SendIbMsg(std::move(sendBuffer));
    std::cout << "Send known participant message" << std::endl;

    bool allParticipantsUp = true;
    for (auto&& participant : _config.simulationSetup.participants)
    {
        allParticipantsUp &= _connectedParticipants.find(participant.id) != _connectedParticipants.end();
    }
    if (allParticipantsUp)
    {
        _allParticipantsUp.set_value();
    }

    return info;
}

void Registry::PeerIsShuttingDown(IVAsioPeer* peer)
{
    _connectedParticipants.erase(peer->GetInfo().participantId);

    if (_connectedParticipants.empty())
    {
        _allParticipantsDown.set_value();
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start registry with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string jsonFilename(argv[1]);

        uint32_t domainId = 42;
        if (argc >= 3)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[2]));
        }

        auto ibConfig = ib::cfg::Config::FromJsonFile(jsonFilename);

        std::cout << "Creating Registry for IB domain=" << domainId << std::endl;

        Registry registry{ibConfig};
        registry.ProvideDomain(domainId);
    }
    catch (const ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
