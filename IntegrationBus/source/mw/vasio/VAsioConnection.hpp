// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <typeinfo>
#include <future>

#include "ib/cfg/Config.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "tuple_tools/for_each.hpp"
#include "tuple_tools/wrapped_tuple.hpp"

#include "MessageBuffer.hpp"

#include "SerdesMw.hpp"
#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesMwVAsio.hpp"
#include "SerdesSimGeneric.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimIo.hpp"
#include "SerdesSimLin.hpp"
#include "SerdesSimFlexray.hpp"
#include "IbMsgTraits.hpp"

#include "IbLink.hpp"
#include "IVAsioPeer.hpp"
#include "VAsioReceiver.hpp"
#include "VAsioTransmitter.hpp"
#include "VAsioMsgKind.hpp"

#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"

#include "asio.hpp"

#ifdef SendMessage
#if SendMessage == SendMessageA
#undef SendMessage
#endif
#endif


namespace ib {
namespace mw {


class VAsioTcpPeer;

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
    VAsioConnection(cfg::Config config, std::string participantName, ParticipantId participantId);
    ~VAsioConnection();

public:
    // ----------------------------------------
    // Operator Implementations
    VAsioConnection& operator=(VAsioConnection& other) = default;
    VAsioConnection& operator=(VAsioConnection&& other) = default;

public:
    // ----------------------------------------
    // Public methods
    void SetLogger(logging::ILogger* logger);
    void JoinDomain(uint32_t domainId);

    template <class IbServiceT>
    void RegisterIbService(const std::string& link, EndpointId endpointId, IbServiceT* service)
    {
        assert(_pendingSubscriptionAcknowledges.empty());
        _receivedAllSubscriptionAcknowledges = std::promise<void>{};
        auto allAcked = _receivedAllSubscriptionAcknowledges.get_future();
        _ioContext.dispatch([this, link, endpointId, service]() {
                    this->RegisterIbServiceImpl<IbServiceT>(link, endpointId, service);
                });
        _logger->Trace("VAsio waiting for subscription acknowledges for IbService {}.", typeid(*service).name());
        allAcked.wait();
        _logger->Trace("VAsio received all subscription acknowledges for IbService {}.", typeid(*service).name());
    }
    template<typename IbMessageT>
    void SendIbMessage(EndpointAddress from, IbMessageT&& msg)
    {
        ExecuteOnIoThread(&VAsioConnection::SendIbMessageImpl<IbMessageT>, from, std::forward<IbMessageT>(msg));
    }

    inline void OnAllMessagesDelivered(std::function<void()> callback)
    {
        callback();
    }
    void FlushSendBuffers() {};

    inline auto Config() const -> const ib::cfg::Config&
    {
        return _config;
    }

    // Temporary Helpers
    void RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback);
    void OnSocketData(IVAsioPeer* from, MessageBuffer&& buffer);

    void AcceptConnectionsOn(asio::ip::tcp::endpoint endpoint);
    void StartIoWorker();

    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> callback);
    void OnPeerShutdown(IVAsioPeer* peer);

    void NotifyShutdown();

private:
    // ----------------------------------------
    // private data types
    template <class MsgT>
    using IbLinkMap = std::map<std::string, std::shared_ptr<IbLink<MsgT>>>;

    template <class MsgT>
    using IbEndpointToLinkMap = std::map<EndpointId, std::shared_ptr<IbLink<MsgT>>>;

    using ParticipantAnnouncementReceiver = std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)>;

    using IbMessageTypes = std::tuple<
        logging::LogMsg,
        sync::NextSimTask,
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
        sim::lin::SendFrameRequest,
        sim::lin::SendFrameHeaderRequest,
        sim::lin::Transmission,
        sim::lin::WakeupPulse,
        sim::lin::ControllerConfig,
        sim::lin::ControllerStatusUpdate,
        sim::lin::FrameResponseUpdate,
        sim::fr::FrMessage,
        sim::fr::FrMessageAck,
        sim::fr::FrSymbol,
        sim::fr::FrSymbolAck,
        sim::fr::CycleStart,
        sim::fr::HostCommand,
        sim::fr::ControllerConfig,
        sim::fr::TxBufferConfigUpdate,
        sim::fr::TxBufferUpdate,
        sim::fr::ControllerStatus,
        sim::fr::PocStatus
    >;

private:
    // ----------------------------------------
    // private methods
    void ReceiveRawIbMessage(MessageBuffer&& buffer);
    void ReceiveSubscriptionAnnouncement(IVAsioPeer* from, MessageBuffer&& buffer);
    void ReceiveSubscriptionAcknowledge(IVAsioPeer* from, MessageBuffer&& buffer);
    void ReceiveRegistryMessage(IVAsioPeer* from, MessageBuffer&& buffer);

    bool TryAddRemoteSubscriber(IVAsioPeer* from, const VAsioMsgSubscriber& subscriber);


    void UpdateParticipantStatusOnConnectionLoss(IVAsioPeer* peer);

    // Registry related send / receive methods
    void ReceiveKnownParticpants(MessageBuffer&& buffer);
    void SendParticipantAnnoucement(IVAsioPeer* peer);
    void ReceiveParticipantAnnouncement(IVAsioPeer* from, MessageBuffer&& buffer);
    void SendParticipantAnnoucementReply(IVAsioPeer* peer);
    void ReceiveParticipantAnnouncementReply(IVAsioPeer* from, MessageBuffer&& buffer);

    template<class IbMessageT>
    auto GetLinkByName(const std::string& linkName) -> std::shared_ptr<IbLink<IbMessageT>>
    {
        auto& ibLink = std::get<IbLinkMap<IbMessageT>>(_ibLinks)[linkName];
        if (!ibLink)
        {
            ibLink = std::make_shared<IbLink<IbMessageT>>(linkName, _logger);
        }
        return ibLink;
    }

    template<class IbMessageT>
    void RegisterIbMsgReceiver(const std::string& linkName, ib::mw::IIbMessageReceiver<IbMessageT>* receiver)
    {
        assert(_logger);

        auto link = GetLinkByName<IbMessageT>(linkName);
        link->AddLocalReceiver(receiver);

        auto vasioReceiver = std::find_if(_vasioReceivers.begin(), _vasioReceivers.end(),
            [&linkName](auto& receiver) {
            return receiver->GetDescriptor().linkName == linkName
                && receiver->GetDescriptor().msgTypeName == IbMsgTraits<IbMessageT>::TypeName();
        });
        if (vasioReceiver == _vasioReceivers.end())
        {
            // we have to subscribe to messages from other peers
            VAsioMsgSubscriber subscriptionInfo;
            subscriptionInfo.receiverIdx = static_cast<uint16_t>(_vasioReceivers.size());
            subscriptionInfo.linkName = linkName;
            subscriptionInfo.msgTypeName = IbMsgTraits<IbMessageT>::TypeName();

            std::unique_ptr<IVAsioReceiver> rawReceiver = std::make_unique<VAsioReceiver<IbMessageT>>(subscriptionInfo, link, _logger);
            _vasioReceivers.emplace_back(std::move(rawReceiver));

            for (auto&& peer : _peers)
            {
                _pendingSubscriptionAcknowledges.emplace_back(peer.get(), subscriptionInfo);
                peer->Subscribe(subscriptionInfo);
            }
        }
    }
    template<class IbMessageT>
    void RegisterIbMsgSender(const std::string& linkName, EndpointId endpointId)
    {
        auto ibLink = GetLinkByName<IbMessageT>(linkName);
        auto&& linkMap = std::get<IbEndpointToLinkMap<IbMessageT>>(_endpointToLinkMap);
        linkMap[endpointId] = ibLink;
    }

    template<class IbServiceT>
    inline void RegisterIbServiceImpl(const std::string& link, EndpointId endpointId, IbServiceT* service)
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

        if (_pendingSubscriptionAcknowledges.empty())
        {
            _receivedAllSubscriptionAcknowledges.set_value();
        }
    }

    template <class IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
    {
        auto& linkMap = std::get<IbEndpointToLinkMap<std::decay_t<IbMessageT>>>(_endpointToLinkMap);
        linkMap[from.endpoint]->DistributeLocalIbMessage(from, std::forward<IbMessageT>(msg));
    }

    template <typename... MethodArgs, typename... Args>
    inline void ExecuteOnIoThread(void (VAsioConnection::*method)(MethodArgs...), Args&&... args)
    {
        if (std::this_thread::get_id() == _ioWorker.get_id())
        {
            (this->*method)(std::forward<Args>(args)...);
        }
        else
        {
            asio::dispatch(_ioContext.get_executor(), [=]() mutable { (this->*method)(std::move(args)...); });
        }
    }
    inline void ExecuteOnIoThread(std::function<void()> function)
    {
        if (std::this_thread::get_id() == _ioWorker.get_id())
        {
            function();
        }
        else
        {
            asio::dispatch(_ioContext.get_executor(), std::move(function));
        }
    }

    // TCP Related
    void AddPeer(std::shared_ptr<VAsioTcpPeer> peer);
    void AcceptNextConnection(asio::ip::tcp::acceptor& acceptor);

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    std::string _participantName;
    ParticipantId _participantId{0};
    logging::ILogger* _logger{nullptr};

    //! \brief Virtual IB links by linkName according to IbConfig.
    util::tuple_tools::wrapped_tuple<IbLinkMap, IbMessageTypes> _ibLinks;
    //! \brief Lookup for sender objects by ID.
    util::tuple_tools::wrapped_tuple<IbEndpointToLinkMap, IbMessageTypes> _endpointToLinkMap;

    std::vector<std::unique_ptr<IVAsioReceiver>> _vasioReceivers;

    // FIXME: generalize the reception of registry data
    std::vector<ParticipantAnnouncementReceiver> _participantAnnouncementReceivers;
    std::vector<std::function<void(IVAsioPeer*)>> _peerShutdownCallbacks;

    // NB: The IO context must be listed before anything socket related.
    asio::io_context _ioContext;

    // NB: peers and acceptors must be listed AFTER the io_context. Otherwise,
    // their destructor will crash!
    std::unique_ptr<IVAsioPeer> _registry{nullptr};
    std::vector<std::shared_ptr<IVAsioPeer>> _peers;
    std::unique_ptr<asio::ip::tcp::acceptor> _tcpAcceptor;

    // After receiving the list of known participants from the registry, we keep
    // track of the sent ParticipantAnnouncements and wait for the corresponding
    // replies.
    std::vector<IVAsioPeer*> _pendingParticipantReplies;
    std::promise<void> _receivedAllParticipantReplies;

    // Keep track of the sent Subscriptions when Registering an IB Service
    std::vector<std::pair<IVAsioPeer*, VAsioMsgSubscriber>> _pendingSubscriptionAcknowledges;
    std::promise<void> _receivedAllSubscriptionAcknowledges;
    
    
    // The worker thread should be the last members in this class. This ensures
    // that no callback is destroyed before the thread finishes.
    std::thread _ioWorker;

    //We violate the strict layering architecture, so that we can cleanly shutdown without false error messages.
    bool _isShuttingDown{false};
};

// ================================================================================
//  Inline Implementations
// ================================================================================


} // mw
} // namespace ib
