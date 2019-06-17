// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"

#include "tuple_tools/for_each.hpp"
#include "tuple_tools/wrapped_tuple.hpp"

#include "MessageBuffer.hpp"

// FIXME: remove includes and dependencies once everything has been templated
#include "SyncMaster.hpp"
#include "SystemMonitor.hpp"
#include "SystemController.hpp"
#include "ParticipantController.hpp"
#include "CanController.hpp"
#include "GenericPublisher.hpp"
#include "GenericSubscriber.hpp"
#include "LogmsgRouter.hpp"

#include "SerdesMw.hpp"
#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimGeneric.hpp"

#include "IVAsioPeer.hpp"
#include "VAsioTcpPeer.hpp"
#include "VAsioReceiver.hpp"
#include "VAsioSender.hpp"
#include "VAsioMessageSubscriber.hpp"
#include "VAsioMsgKind.hpp"

#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"

#include "asio.hpp"


#define DefineIbMsgTraits(Namespace, MsgName) template<> struct IbMsgTraits<Namespace::MsgName> { static constexpr const char* TypeName() { return #Namespace "::" #MsgName; } };

#define DefineRegisterServiceMethod(IbServiceT) template<> inline void VAsioConnection::RegisterIbService<IbServiceT>(const std::string& link, EndpointId endpointId, IbServiceT* service) { RegisterIbService__<IbServiceT>(link, endpointId, service); }
#define DefineSendIbMessageMethod(IbMsgT) template<> inline void VAsioConnection::SendIbMessageImpl<IbMsgT>(EndpointAddress from, const IbMsgT& msg) { SendIbMessageImpl__(from, msg); }


namespace ib {
namespace mw {

template <class MsgT> struct IbMsgTraits;

DefineIbMsgTraits(ib::mw::logging, LogMsg)
DefineIbMsgTraits(ib::mw::sync, Tick)
DefineIbMsgTraits(ib::mw::sync, TickDone)
DefineIbMsgTraits(ib::mw::sync, QuantumRequest)
DefineIbMsgTraits(ib::mw::sync, QuantumGrant)
DefineIbMsgTraits(ib::mw::sync, ParticipantCommand)
DefineIbMsgTraits(ib::mw::sync, SystemCommand)
DefineIbMsgTraits(ib::mw::sync, ParticipantStatus)
DefineIbMsgTraits(ib::sim::can, CanMessage)
DefineIbMsgTraits(ib::sim::can, CanTransmitAcknowledge)
DefineIbMsgTraits(ib::sim::generic, GenericMessage)


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
    ~VAsioConnection();

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

    template<class IbMessageT>
    void SendIbMessageImpl__(EndpointAddress from, IbMessageT&& msg);

    //template<class IbMessageT>
    //void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);
    template<class IbMessageT>
    inline void SendIbMessageImpl(EndpointAddress /*from*/, const IbMessageT& /*msg*/) {}

    void OnAllMessagesDelivered(std::function<void()> callback) {};
    void FlushSendBuffers() {};

    // Temporary Helpers
    void OnSocketData(MessageBuffer&& buffer, IVAsioPeer* peer);

private:
    // ----------------------------------------
    // private datatypes
    template <class MsgT>
    struct IbLink
    {
        std::shared_ptr<VAsioReceiver<MsgT>> receiver;
        std::shared_ptr<VAsioSender<MsgT>>   sender;
    };

    template <class MsgT>
    using IbLinkMap = std::map<std::string, IbLink<MsgT>>;

    template <class MsgT>
    using IbSenderMap = std::map<EndpointId, std::shared_ptr<VAsioSender<MsgT>>>;

    using IbMessageTypes = std::tuple<
        logging::LogMsg,
        sync::Tick,
        sync::TickDone,
        sync::QuantumRequest,
        sync::QuantumGrant,
        sync::SystemCommand,
        sync::ParticipantCommand,
        sync::ParticipantStatus,
        sim::can::CanMessage,
        sim::can::CanTransmitAcknowledge,
        sim::generic::GenericMessage
    >;

private:
    // ----------------------------------------
    // private methods
    void ReceiveRawIbMessage(MessageBuffer&& buffer);
    void ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer);

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

    //! \brief Virtual IB links by linkName according to IbConfig.
    // An std::tuple<IbLinkMap<IbMsgT>...> for all supported IbMessageTypes
    util::tuple_tools::wrapped_tuple<IbLinkMap, IbMessageTypes> _ibLinks;

    //! \brief Lookup for sender objects by ID.
    // An std::tuple<IbSenderMap<IbMsgT>...> for all supported IbMessageTypes
    util::tuple_tools::wrapped_tuple<IbSenderMap, IbMessageTypes> _endpointToSenderMap;


    std::vector<std::shared_ptr<IVAsioReceiver>> _rawMsgReceivers;

    std::vector<std::shared_ptr<IVAsioPeer>> _peers;

    asio::io_context _ioContext;
    std::thread _ioWorker;
    std::unique_ptr<asio::ip::tcp::acceptor> _tcpAcceptor;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IbMessageT>
bool VAsioConnection::TryAddSubscriber(const VAsioMsgSubscriber& subscriber, IVAsioPeer* peer)
{
    if (subscriber.msgTypeName != IbMsgTraits<IbMessageT>::TypeName())
        return false;

    auto&& ibLink = std::get<IbLinkMap<IbMessageT>>(_ibLinks)[subscriber.linkName];
    if (!ibLink.sender)
    {
        ibLink.sender = std::make_shared<VAsioSender<IbMessageT>>();
    }
    ibLink.sender->AddSubscriber(subscriber.receiverIdx, peer);

    std::cout << "INFO: Registered subscriber for: [" << subscriber.linkName << "] " << IbMsgTraits<IbMessageT>::TypeName() << "\n";
    return true;
}

DefineRegisterServiceMethod(ib::mw::logging::LogmsgRouter)
DefineRegisterServiceMethod(ib::mw::sync::ParticipantController)
DefineRegisterServiceMethod(ib::mw::sync::SyncMaster)
DefineRegisterServiceMethod(ib::mw::sync::SystemMonitor)
DefineRegisterServiceMethod(ib::mw::sync::SystemController)
DefineRegisterServiceMethod(ib::sim::can::CanController)
DefineRegisterServiceMethod(ib::sim::generic::GenericPublisher)
DefineRegisterServiceMethod(ib::sim::generic::GenericSubscriber)
    
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
    auto&& ibLink = std::get<IbLinkMap<IbMessageT>>(_ibLinks)[link];
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
    auto&& ibLink = std::get<IbLinkMap<IbMessageT>>(_ibLinks)[link];

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

DefineSendIbMessageMethod(logging::LogMsg)
DefineSendIbMessageMethod(sync::Tick)
DefineSendIbMessageMethod(sync::TickDone)
DefineSendIbMessageMethod(sync::QuantumRequest)
DefineSendIbMessageMethod(sync::QuantumGrant)
DefineSendIbMessageMethod(sync::ParticipantCommand)
DefineSendIbMessageMethod(sync::SystemCommand)
DefineSendIbMessageMethod(sync::ParticipantStatus)
DefineSendIbMessageMethod(sim::can::CanMessage)
DefineSendIbMessageMethod(sim::can::CanTransmitAcknowledge)
DefineSendIbMessageMethod(sim::generic::GenericMessage)


} // mw
} // namespace ib
