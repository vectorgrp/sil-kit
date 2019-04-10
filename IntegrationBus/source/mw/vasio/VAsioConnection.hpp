// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"

#include "tuple_tools/for_each.hpp"

#include "MessageBuffer.hpp"

// FIXME: remove includes and dependencies once everything has been templated
#include "SyncMaster.hpp"
#include "SystemMonitor.hpp"
#include "SystemController.hpp"
#include "ParticipantController.hpp"
#include "CanController.hpp"

#include "SerdesMw.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesSimCan.hpp"

#include "IVAsioPeer.hpp"
#include "VAsioTcpPeer.hpp"
#include "VAsioReceiver.hpp"
#include "VAsioSender.hpp"
#include "VAsioMessageSubscriber.hpp"
#include "VAsioMsgKind.hpp"

#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"

#include "asio.hpp"


namespace ib {
namespace mw {

template <class MsgT> struct IbMsgTraits;

#define DefineIbMsgTraits(Namespace, MsgName) template<> struct IbMsgTraits<Namespace::MsgName> { static constexpr const char* TypeName() { return #Namespace "::" #MsgName; } };

DefineIbMsgTraits(ib::mw::sync, Tick)
DefineIbMsgTraits(ib::mw::sync, TickDone)
DefineIbMsgTraits(ib::mw::sync, QuantumRequest)
DefineIbMsgTraits(ib::mw::sync, QuantumGrant)
DefineIbMsgTraits(ib::mw::sync, ParticipantCommand)
DefineIbMsgTraits(ib::mw::sync, SystemCommand)
DefineIbMsgTraits(ib::mw::sync, ParticipantStatus)
DefineIbMsgTraits(ib::sim::can, CanMessage)
DefineIbMsgTraits(ib::sim::can, CanTransmitAcknowledge)

#define DefineRegisterServiceMethod(IbServiceT) template<> void RegisterIbService<IbServiceT>(const std::string& link, EndpointId endpointId, IbServiceT* service) { RegisterIbService__<IbServiceT>(link, endpointId, service); }
#define DefineSendIbMessageMethod(IbMsgT) template<> void SendIbMessageImpl<IbMsgT>(EndpointAddress from, const IbMsgT& msg) { SendIbMessageImpl__(from, msg); }

template <class MsgT>
struct VAsioLink
{
    std::shared_ptr<VAsioReceiver<MsgT>> receiver;
    std::shared_ptr<VAsioSender<MsgT>>   sender;
};

class VAsioConnection
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioConnection() = default;
    VAsioConnection(const VAsioConnection&) = default;
    VAsioConnection(VAsioConnection&&) = default;
    VAsioConnection(cfg::Config config, std::string participantName);

public:
    // ----------------------------------------
    // Operator Implementations
    VAsioConnection& operator=(VAsioConnection& other) = default;
    VAsioConnection& operator=(VAsioConnection&& other) = default;

public:
    // ----------------------------------------
    // Public methods
    void joinDomain(uint32_t domainId);

    template <class IbServiceT>
    inline void RegisterIbService__(const std::string& link, EndpointId endpointId, IbServiceT* service);

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& /*link*/, EndpointId /*endpointId*/, IbServiceT* /*receiver*/) {}
    DefineRegisterServiceMethod(ib::mw::sync::ParticipantController)
    DefineRegisterServiceMethod(ib::mw::sync::SyncMaster)
    DefineRegisterServiceMethod(ib::mw::sync::SystemMonitor)
    DefineRegisterServiceMethod(ib::mw::sync::SystemController)
    DefineRegisterServiceMethod(ib::sim::can::CanController)

    template<class IbMessageT>
    void SendIbMessageImpl__(EndpointAddress from, IbMessageT&& msg);

    //template<class IbMessageT>
    //void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);
    template<class IbMessageT>
    void SendIbMessageImpl(EndpointAddress /*from*/, const IbMessageT& /*msg*/) {}
    DefineSendIbMessageMethod(sync::Tick)
    DefineSendIbMessageMethod(sync::TickDone)
    DefineSendIbMessageMethod(sync::QuantumRequest)
    DefineSendIbMessageMethod(sync::QuantumGrant)
    DefineSendIbMessageMethod(sync::ParticipantCommand)
    DefineSendIbMessageMethod(sync::SystemCommand)
    DefineSendIbMessageMethod(sync::ParticipantStatus)
    DefineSendIbMessageMethod(sim::can::CanMessage)
    DefineSendIbMessageMethod(sim::can::CanTransmitAcknowledge)


    void WaitForMessageDelivery() {};
    void FlushSendBuffers() {};

    void Run();
    void Stop() {};

    // Temporary Helpers
    inline void OnSocketData(MessageBuffer&& buffer, IVAsioPeer* peer);


private:
    // ----------------------------------------
    // private datatypes
    template <class MsgT>
    using IbSenderMap = std::map<EndpointId, std::shared_ptr<VAsioSender<MsgT>>>;

private:
    // ----------------------------------------
    // private methods
    inline void ReceiveRawIbMessage(MessageBuffer&& buffer);
    inline void ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer);

    template<class IbMessageT>
    bool TryAddSubscriber(const VAsioMsgSubscriber& subscriber, IVAsioPeer* peer);

    template<class IbMessageT>
    void RegisterIbMsgReceiver(const std::string& link, ib::mw::IIbMessageReceiver<IbMessageT>* receiver);
    template<class IbMessageT>
    void RegisterIbMsgSender(const std::string& link, EndpointId endpointId);

    // TCP Related
    void AcceptConnection();
    void AddPeer(std::shared_ptr<VAsioTcpPeer> peer);

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    std::string _participantName;
    ParticipantId _participantId{0};

    std::map<std::string, std::tuple<
        VAsioLink<sync::Tick>,
        VAsioLink<sync::TickDone>,
        VAsioLink<sync::QuantumRequest>,
        VAsioLink<sync::QuantumGrant>,
        VAsioLink<sync::SystemCommand>,
        VAsioLink<sync::ParticipantCommand>,
        VAsioLink<sync::ParticipantStatus>,
        VAsioLink<sim::can::CanMessage>,
        VAsioLink<sim::can::CanTransmitAcknowledge>
    >> _links;

    std::tuple<
        IbSenderMap<sync::Tick>,
        IbSenderMap<sync::TickDone>,
        IbSenderMap<sync::QuantumRequest>,
        IbSenderMap<sync::QuantumGrant>,
        IbSenderMap<sync::SystemCommand>,
        IbSenderMap<sync::ParticipantCommand>,
        IbSenderMap<sync::ParticipantStatus>,
        IbSenderMap<sim::can::CanMessage>,
        IbSenderMap<sim::can::CanTransmitAcknowledge>
    > _endpointToSenderMap;

    std::vector<std::shared_ptr<IVAsioReceiver>> _rawMsgReceivers;

    std::vector<std::shared_ptr<IVAsioPeer>> _peers;

    asio::io_context _ioContext;
    std::unique_ptr<asio::ip::tcp::acceptor> _tcpAcceptor;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
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

void VAsioConnection::ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer)
{
    VAsioMsgSubscriber subscriber;
    buffer >> subscriber;

    if (TryAddSubscriber<sync::Tick>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::TickDone>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::QuantumRequest>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::QuantumGrant>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::ParticipantCommand>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::SystemCommand>(subscriber, peer)) return;
    if (TryAddSubscriber<sync::ParticipantStatus>(subscriber, peer)) return;
    if (TryAddSubscriber<sim::can::CanMessage>(subscriber, peer)) return;
    if (TryAddSubscriber<sim::can::CanTransmitAcknowledge>(subscriber, peer)) return;

    std::cout << "Cannot register subscription for: " << subscriber.linkName << " - " << subscriber.msgTypeName << "\n";
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

template<class IbMessageT>
bool VAsioConnection::TryAddSubscriber(const VAsioMsgSubscriber& subscriber, IVAsioPeer* peer)
{
    if (subscriber.msgTypeName == IbMsgTraits<IbMessageT>::TypeName())
    {
        std::cout << "Adding Subscriber on Link " << subscriber.linkName << " for " << IbMsgTraits<IbMessageT>::TypeName() << "\n";
        auto&& ibLink = std::get<VAsioLink<IbMessageT>>(_links[subscriber.linkName]);
        if (!ibLink.sender)
        {
            ibLink.sender = std::make_shared<VAsioSender<IbMessageT>>();
        }
        ibLink.sender->AddSubscriber(subscriber.receiverIdx, peer);
        return true;
    }
    return false;
}

template <class IbServiceT>
void VAsioConnection::RegisterIbService__(const std::string& link, EndpointId endpointId, IbServiceT* service)
{
    typename IbServiceT::IbReceiveMessagesTypes receiveMessageTypes{};
    typename IbServiceT::IbSendMessagesTypes sendMessageTypes{};

    util::tuple_tools::for_each(receiveMessageTypes,
        [this, &link, service](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            this->RegisterIbMsgReceiver<IbMessageT>(link, service);
        }
    );

    util::tuple_tools::for_each(sendMessageTypes,
        [this, &link, &endpointId](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            this->RegisterIbMsgSender<IbMessageT>(link, endpointId);
        }
    );

}


template<class IbMessageT>
void VAsioConnection::RegisterIbMsgReceiver(const std::string& link, ib::mw::IIbMessageReceiver<IbMessageT>* receiver)
{
    auto&& ibLink = std::get<VAsioLink<IbMessageT>>(_links[link]);
    if (!ibLink.receiver)
    {
        ibLink.receiver = std::make_shared<VAsioReceiver<IbMessageT>>();
        ibLink.receiver->SetReceiverIdx(static_cast<uint16_t>(_rawMsgReceivers.size()));
        ibLink.receiver->GetDescriptor().linkName = link;
        ibLink.receiver->GetDescriptor().msgTypeName = IbMsgTraits<IbMessageT>::TypeName();
        ibLink.receiver->AddReceiver(receiver);

        _rawMsgReceivers.push_back(ibLink.receiver);
        for (auto&& peer : _peers)
        {
            peer->Subscribe(ibLink.receiver->GetDescriptor());
        }
    }
    else
    {
        ibLink.receiver->AddReceiver(receiver);
    }
}

template<class IbMessageT>
void VAsioConnection::RegisterIbMsgSender(const std::string& link, EndpointId endpointId)
{
    auto&& ibLink = std::get<VAsioLink<IbMessageT>>(_links[link]);

    if (!ibLink.sender)
    {
        ibLink.sender = std::make_shared<VAsioSender<IbMessageT>>();
    }
    auto&& senderMap = std::get<IbSenderMap<IbMessageT>>(_endpointToSenderMap);
    senderMap[endpointId] = ibLink.sender;
}


template <class IbMessageT>
void VAsioConnection::SendIbMessageImpl__(EndpointAddress from, IbMessageT&& msg)
{
    auto&& senderMap = std::get<IbSenderMap<std::decay_t<IbMessageT>>>(_endpointToSenderMap);
    senderMap[from.endpoint]->SendIbMessage(from, std::forward<IbMessageT>(msg));
}



} // mw
} // namespace ib
