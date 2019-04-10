// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace mw {

namespace tt = util::tuple_tools;

using asio::ip::tcp;

VAsioConnection::VAsioConnection(cfg::Config config, std::string participantName)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{get_by_name(_config.simulationSetup.participants, _participantName).id}
{
}

void VAsioConnection::Run()
{
    using namespace std::chrono_literals;
    static int callCount = 0;
    if (callCount == 0)
    {
        _ioContext.run_for(2s);
        callCount++;
    }
    else
    {
        _ioContext.run();
    }
}

void VAsioConnection::joinDomain(uint32_t domainId)
{
    assert(!_tcpAcceptor);

    auto toPort = [domainId](auto participantId)
    {
        return static_cast<uint16_t>(1000 + domainId * 10 + participantId);
    };

    std::cout << "Listening on port " << toPort(_participantId) << "\n";
    _tcpAcceptor = std::make_unique<asio::ip::tcp::acceptor>(_ioContext, tcp::endpoint(tcp::v4(), toPort(_participantId)));
    AcceptConnection();


    for (ParticipantId otherId = 1u; otherId < _participantId; otherId++)
    {
        std::cout << "Connecting to port " << toPort(otherId) << "\n";
        auto peer = std::make_unique<VAsioTcpPeer>(_tcpAcceptor->get_executor().context(), this);
        peer->Socket().connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), toPort(otherId)));
        AddPeer(std::move(peer));
    }
}

void VAsioConnection::AcceptConnection()
{
    assert(_tcpAcceptor);
    auto newConnection = std::make_shared<VAsioTcpPeer>(_tcpAcceptor->get_executor().context(), this);

    _tcpAcceptor->async_accept(newConnection->Socket(),
        [this, newConnection](const asio::error_code& error) mutable
        {
            if (!error)
            {
                std::cout << "New Connection!! WOOHOO!!!\n";
                AddPeer(std::move(newConnection));
            }
            AcceptConnection();
        }
    );
}

void VAsioConnection::AddPeer(std::shared_ptr<VAsioTcpPeer> peer)
{
    for (auto&& localReceiver : _rawMsgReceivers)
    {
        peer->Subscribe(localReceiver->GetDescriptor());
    }
    peer->StartAsyncRead();

    _peers.emplace_back(std::move(peer));
}


} // namespace mw
} // namespace ib
