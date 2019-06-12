// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioConnection.hpp"

#include "ib/cfg/string_utils.hpp"

namespace ib {
namespace mw {

namespace tt = util::tuple_tools;

template <class T> struct Zero { using Type = T; };

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
                std::cout << "Established new Connection!\n";
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


void VAsioConnection::OnSocketData(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    VAsioMsgKind msgKind;
    buffer >> msgKind;
    switch (msgKind)
    {
    case VAsioMsgKind::Invalid:
        std::cerr << "WARNING: Received message with VAsioMsgKind::Invalid\n";
        break;
    case VAsioMsgKind::AnnounceSubscription:
        return ReceiveSubscriptionAnnouncement(std::move(buffer), peer);
        break;
    case VAsioMsgKind::IbMwMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    case VAsioMsgKind::IbSimMsg:
        return ReceiveRawIbMessage(std::move(buffer));
    }
}

void VAsioConnection::ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    VAsioMsgSubscriber subscriber;
    buffer >> subscriber;

    bool wasAdded = false;

    tt::wrapped_tuple<Zero, IbMessageTypes> supportedTypes;
    tt::for_each(supportedTypes, [&](auto zero) {

        using IbMessageT = typename decltype(zero)::Type;
        wasAdded |= this->TryAddSubscriber<IbMessageT>(subscriber, peer);

        });

    if (!wasAdded)
    {
        std::cout << "WARNING: Cannot register subscriber for: [" << subscriber.linkName << "] " << subscriber.msgTypeName << "\n";
    }
}

void VAsioConnection::ReceiveRawIbMessage(MessageBuffer&& buffer)
{
    uint16_t receiverIdx;
    buffer >> receiverIdx;
    if (receiverIdx >= _rawMsgReceivers.size())
    {
        std::cerr << "WARNING: Ignoring RawIbMessage for unknown receiverIdx=" << receiverIdx << "\n";
        return;
    }
    _rawMsgReceivers[receiverIdx]->ReceiveMsg(std::move(buffer));
}



} // namespace mw
} // namespace ib
