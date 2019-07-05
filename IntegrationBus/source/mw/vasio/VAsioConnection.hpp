// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <future>

#include "ib/cfg/Config.hpp"

#include "tuple_tools/for_each.hpp"
#include "tuple_tools/wrapped_tuple.hpp"

#include "MessageBuffer.hpp"

// FIXME: remove includes and dependencies once everything has been templated
#include "SyncMaster.hpp"
#include "SystemMonitor.hpp"
#include "SystemController.hpp"
#include "ParticipantController.hpp"
#include "GenericPublisher.hpp"
#include "GenericSubscriber.hpp"
#include "LogmsgRouter.hpp"
#include "CanController.hpp"
#include "EthController.hpp"
#include "InPort.hpp"
#include "OutPort.hpp"
#include "LinController.hpp"

#include "SerdesMw.hpp"
#include "SerdesMwRegistry.hpp"
#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesSimGeneric.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimIo.hpp"
#include "SerdesSimLin.hpp"

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
DefineIbMsgTraits(ib::sim::generic, GenericMessage)
DefineIbMsgTraits(ib::sim::can, CanMessage)
DefineIbMsgTraits(ib::sim::can, CanTransmitAcknowledge)
DefineIbMsgTraits(ib::sim::can, CanControllerStatus)
DefineIbMsgTraits(ib::sim::can, CanConfigureBaudrate)
DefineIbMsgTraits(ib::sim::can, CanSetControllerMode)
DefineIbMsgTraits(ib::sim::eth, EthMessage)
DefineIbMsgTraits(ib::sim::eth, EthTransmitAcknowledge)
DefineIbMsgTraits(ib::sim::eth, EthStatus)
DefineIbMsgTraits(ib::sim::eth, EthSetMode)
DefineIbMsgTraits(ib::sim::io, AnalogIoMessage)
DefineIbMsgTraits(ib::sim::io, DigitalIoMessage)
DefineIbMsgTraits(ib::sim::io, PatternIoMessage)
DefineIbMsgTraits(ib::sim::io, PwmIoMessage)
DefineIbMsgTraits(ib::sim::lin, LinMessage)
DefineIbMsgTraits(ib::sim::lin, RxRequest)
DefineIbMsgTraits(ib::sim::lin, TxAcknowledge)
DefineIbMsgTraits(ib::sim::lin, WakeupRequest)
DefineIbMsgTraits(ib::sim::lin, ControllerConfig)
DefineIbMsgTraits(ib::sim::lin, SlaveConfiguration)
DefineIbMsgTraits(ib::sim::lin, SlaveResponse)

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
    void JoinDomain(uint32_t domainId);

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
    void RegisterNewPeerCallback(std::function<void()> callback);

    // Temporary Helpers
    void OnSocketData(MessageBuffer&& buffer, IVAsioPeer* peer);
    virtual void PeerIsShuttingDown(IVAsioPeer* /*peer*/) {};

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
        sim::generic::GenericMessage,
        sim::can::CanMessage,
        sim::can::CanTransmitAcknowledge,
        sim::can::CanControllerStatus,
        sim::can::CanConfigureBaudrate,
        sim::can::CanSetControllerMode,
        sim::eth::EthMessage,
        sim::eth::EthTransmitAcknowledge,
        sim::eth::EthStatus,
        sim::eth::EthSetMode,
        sim::io::AnalogIoMessage,
        sim::io::DigitalIoMessage,
        sim::io::PatternIoMessage,
        sim::io::PwmIoMessage,
        sim::lin::LinMessage,
        sim::lin::RxRequest,
        sim::lin::TxAcknowledge,
        sim::lin::WakeupRequest,
        sim::lin::ControllerConfig,
        sim::lin::SlaveConfiguration,
        sim::lin::SlaveResponse
    >;

protected:
    void StartIoWorker();
    void AcceptConnection();
    virtual auto ReceiveParticipantAnnoucement(MessageBuffer&& buffer, IVAsioPeer* peer) -> VAsioPeerInfo;

private:
    // ----------------------------------------
    // private methods
    void ReceiveRawIbMessage(MessageBuffer&& buffer);
    void ReceiveSubscriptionAnnouncement(MessageBuffer&& buffer, IVAsioPeer* peer);
    void ReceiveRegistryMessage(MessageBuffer&& buffer, IVAsioPeer* peer);

    void ReceiveKnownParticpants(MessageBuffer&& buffer);
    void SendParticipantAnnoucement(IVAsioPeer* peer);
    void ReceiveSubscriptionSentEvent();

    template<class IbMessageT>
    bool TryAddSubscriber(const VAsioMsgSubscriber& subscriber, IVAsioPeer* peer);

    template<class IbMessageT>
    void RegisterIbMsgReceiver(const std::string& link, ib::mw::IIbMessageReceiver<IbMessageT>* receiver);
    template<class IbMessageT>
    void RegisterIbMsgSender(const std::string& link, EndpointId endpointId);

    // TCP Related
    void AddPeer(std::shared_ptr<VAsioTcpPeer> peer);

protected:
    asio::io_context _ioContext;
    std::unique_ptr<asio::ip::tcp::acceptor> _tcpAcceptor;
    std::vector<std::shared_ptr<IVAsioPeer>> _peers;
    cfg::Config _config;

private:
    // ----------------------------------------
    // private members
    std::string _participantName;
    ParticipantId _participantId{0};

    //! \brief Virtual IB links by linkName according to IbConfig.
    // An std::tuple<IbLinkMap<IbMsgT>...> for all supported IbMessageTypes
    util::tuple_tools::wrapped_tuple<IbLinkMap, IbMessageTypes> _ibLinks;

    //! \brief Lookup for sender objects by ID.
    // An std::tuple<IbSenderMap<IbMsgT>...> for all supported IbMessageTypes
    util::tuple_tools::wrapped_tuple<IbSenderMap, IbMessageTypes> _endpointToSenderMap;

    std::vector<std::shared_ptr<IVAsioReceiver>> _rawMsgReceivers;

    std::unique_ptr<IVAsioPeer> _registry{nullptr};

    std::thread _ioWorker;

    std::function<void()> _newPeerCallback{nullptr};
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
DefineRegisterServiceMethod(ib::sim::generic::GenericPublisher)
DefineRegisterServiceMethod(ib::sim::generic::GenericSubscriber)
DefineRegisterServiceMethod(ib::sim::can::CanController)
DefineRegisterServiceMethod(ib::sim::eth::EthController)
DefineRegisterServiceMethod(ib::sim::io::InPort<ib::sim::io::AnalogIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::InPort<ib::sim::io::DigitalIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::InPort<ib::sim::io::PatternIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::InPort<ib::sim::io::PwmIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::OutPort<ib::sim::io::AnalogIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::OutPort<ib::sim::io::DigitalIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::OutPort<ib::sim::io::PatternIoMessage>)
DefineRegisterServiceMethod(ib::sim::io::OutPort<ib::sim::io::PwmIoMessage>)
DefineRegisterServiceMethod(ib::sim::lin::LinController)

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
DefineSendIbMessageMethod(sim::generic::GenericMessage)
DefineSendIbMessageMethod(sim::can::CanMessage)
DefineSendIbMessageMethod(sim::can::CanTransmitAcknowledge)
DefineSendIbMessageMethod(sim::can::CanControllerStatus)
DefineSendIbMessageMethod(sim::can::CanConfigureBaudrate)
DefineSendIbMessageMethod(sim::can::CanSetControllerMode)
DefineSendIbMessageMethod(sim::eth::EthMessage)
DefineSendIbMessageMethod(sim::eth::EthTransmitAcknowledge)
DefineSendIbMessageMethod(sim::eth::EthStatus)
DefineSendIbMessageMethod(sim::eth::EthSetMode)
DefineSendIbMessageMethod(sim::io::AnalogIoMessage)
DefineSendIbMessageMethod(sim::io::DigitalIoMessage)
DefineSendIbMessageMethod(sim::io::PatternIoMessage)
DefineSendIbMessageMethod(sim::io::PwmIoMessage)
DefineSendIbMessageMethod(sim::lin::LinMessage)
DefineSendIbMessageMethod(sim::lin::RxRequest)
DefineSendIbMessageMethod(sim::lin::TxAcknowledge)
DefineSendIbMessageMethod(sim::lin::WakeupRequest)
DefineSendIbMessageMethod(sim::lin::ControllerConfig)
DefineSendIbMessageMethod(sim::lin::SlaveConfiguration)
DefineSendIbMessageMethod(sim::lin::SlaveResponse)


} // mw
} // namespace ib
