// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <typeinfo>
#include <future>

#include "ParticipantConfiguration.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "tuple_tools/for_each.hpp"
#include "tuple_tools/wrapped_tuple.hpp"

#include "MessageBuffer.hpp"

#include "SerdesMw.hpp"
#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesMwVAsio.hpp"
#include "SerdesSimData.hpp"
#include "SerdesSimRpc.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimLin.hpp"
#include "SerdesSimFlexray.hpp"
#include "SerdesMwService.hpp"

#include "IbLink.hpp"
#include "IVAsioPeer.hpp"
#include "VAsioReceiver.hpp"
#include "VAsioTransmitter.hpp"
#include "VAsioMsgKind.hpp"
#include "IIbServiceEndpoint.hpp"
#include "traits/IbMsgTraits.hpp"
#include "traits/IbServiceTraits.hpp"

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
    VAsioConnection(const VAsioConnection&) = delete; //clang warning: this is implicity deleted by asio::io_context
    VAsioConnection(VAsioConnection&&) = delete; // ditto asio::io_context
    VAsioConnection(std::shared_ptr<ib::cfg::v1::datatypes::ParticipantConfiguration> config, std::string participantName, ParticipantId participantId);
    ~VAsioConnection();

public:
    // ----------------------------------------
    // Operator Implementations
    VAsioConnection& operator=(VAsioConnection& other) = delete; // also implicitly deleted by asio::io_context
    VAsioConnection& operator=(VAsioConnection&& other) = delete;

public:
    // ----------------------------------------
    // Public methods
    void SetLogger(logging::ILogger* logger);
    void JoinDomain(uint32_t domainId);

    template <class IbServiceT>
    void RegisterIbService(const std::string& link, EndpointId endpointId, IbServiceT* service)
    {
        std::future<void> allAcked;
        if (!IbServiceTraits<IbServiceT>::UseAsyncRegistration())
        {
            assert(_pendingSubscriptionAcknowledges.empty());
            _receivedAllSubscriptionAcknowledges = std::promise<void>{};
            allAcked = _receivedAllSubscriptionAcknowledges.get_future();
        }

        _ioContext.dispatch([this, link, endpointId, service]() {
            this->RegisterIbServiceImpl<IbServiceT>(link, endpointId, service);
        });

        if (!IbServiceTraits<IbServiceT>::UseAsyncRegistration())
        {
            _logger->Trace("VAsio waiting for subscription acknowledges for IbService {}.", typeid(*service).name());
            allAcked.wait();
            _logger->Trace("VAsio received all subscription acknowledges for IbService {}.", typeid(*service).name());
        }
    }

    template <class IbServiceT>
    void SetHistoryLengthForLink(const std::string& networkName, size_t historyLength, IbServiceT* /*service*/)
    {
        // NB: Dummy IbServiceT* is fed in here to deduce IbServiceT, as it is only used in 'typename IbServiceT',
        // which is not sufficient to get the type for some compilers (e.g. Clang)
        typename IbServiceT::IbSendMessagesTypes sendMessageTypes{};

        util::tuple_tools::for_each(sendMessageTypes, [this, &networkName, historyLength](auto&& ibMessage) {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            auto link = this->GetLinkByName<IbMessageT>(networkName); 
            link->SetHistoryLength(historyLength);
        });
    }

    template<typename IbMessageT>
    void SendIbMessage(const IIbServiceEndpoint* from, IbMessageT&& msg)
    {
        ExecuteOnIoThread(&VAsioConnection::SendIbMessageImpl<IbMessageT>, from, std::forward<IbMessageT>(msg));
    }

    template<typename IbMessageT>
    void SendIbMessage(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg)
    {
        ExecuteOnIoThread(&VAsioConnection::SendIbMessageToTargetImpl<IbMessageT>, from, targetParticipantName, std::forward<IbMessageT>(msg));
    }

    inline void OnAllMessagesDelivered(std::function<void()> callback)
    {
        callback();
    }
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> function)
    {
        asio::post(_ioContext.get_executor(), std::move(function));
    }

    inline auto Config() const -> std::shared_ptr<ib::cfg::v1::datatypes::ParticipantConfiguration>
    {
        return _config;
    }

    // Temporary Helpers
    void RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback);
    void OnSocketData(IVAsioPeer* from, MessageBuffer&& buffer);

    template<typename EndpointT>
    void AcceptConnectionsOn(EndpointT endpoint);
    void AcceptLocalConnections(uint32_t domainId);

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
    using IbServiceToReceiverMap = std::map<std::string, IIbMessageReceiver<MsgT>*>;
    template <class MsgT>
    using IbServiceToLinkMap = std::map<std::string, std::shared_ptr<IbLink<MsgT>>>;

    using ParticipantAnnouncementReceiver = std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)>;

    using IbMessageTypes = std::tuple<
        logging::LogMsg,
        sync::NextSimTask,
        sync::SystemCommand,
        sync::ParticipantCommand,
        sync::ParticipantStatus,
        sync::ExpectedParticipants,
        sim::data::DataMessage,
        sim::rpc::FunctionCall,
        sim::rpc::FunctionCallResponse,
        sim::can::CanMessage,
        sim::can::CanTransmitAcknowledge,
        sim::can::CanControllerStatus,
        sim::can::CanConfigureBaudrate,
        sim::can::CanSetControllerMode,
        sim::eth::EthMessage,
        sim::eth::EthTransmitAcknowledge,
        sim::eth::EthStatus,
        sim::eth::EthSetMode,
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
        sim::fr::PocStatus,
        mw::service::ServiceAnnouncement,
        mw::service::ServiceDiscoveryEvent
    >;

private:
    // ----------------------------------------
    // private methods
    void ReceiveRawIbMessage(IVAsioPeer* from, MessageBuffer&& buffer);
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

    void AddParticipantToLookup(const std::string& participantName);
    const std::string& GetParticipantFromLookup(const std::uint64_t participantId) const;

    template<class IbMessageT>
    auto GetLinkByName(const std::string& networkName) -> std::shared_ptr<IbLink<IbMessageT>>
    {
        auto& ibLink = std::get<IbLinkMap<IbMessageT>>(_ibLinks)[networkName];
        if (!ibLink)
        {
            ibLink = std::make_shared<IbLink<IbMessageT>>(networkName, _logger);
        }
        return ibLink;
    }

    template<class IbMessageT, class IbServiceT>
    void RegisterIbMsgReceiver(const std::string& networkName, ib::mw::IIbMessageReceiver<IbMessageT>* receiver)
    {
        assert(_logger);

        auto link = GetLinkByName<IbMessageT>(networkName);
        link->AddLocalReceiver(receiver);

        auto vasioReceiver = std::find_if(_vasioReceivers.begin(), _vasioReceivers.end(),
            [&networkName](auto& msgReceiver) {
            return msgReceiver->GetDescriptor().networkName == networkName
                && msgReceiver->GetDescriptor().msgTypeName == IbMsgTraits<IbMessageT>::TypeName();
        });
        if (vasioReceiver == _vasioReceivers.end())
        {
            // we have to subscribe to messages from other peers
            VAsioMsgSubscriber subscriptionInfo;
            subscriptionInfo.receiverIdx = static_cast<uint16_t>(_vasioReceivers.size());
            subscriptionInfo.networkName = networkName;
            subscriptionInfo.msgTypeName = IbMsgTraits<IbMessageT>::TypeName();

            std::unique_ptr<IVAsioReceiver> rawReceiver = std::make_unique<VAsioReceiver<IbMessageT>>(subscriptionInfo, link, _logger);
            auto* serviceEndpointPtr = dynamic_cast<IIbServiceEndpoint*>(rawReceiver.get());
            ServiceDescriptor tmpServiceDescriptor(dynamic_cast<mw::IIbServiceEndpoint&>(*receiver).GetServiceDescriptor());
            tmpServiceDescriptor.SetParticipantName(_participantName);
            //Copy the Service Endpoint Id
            serviceEndpointPtr->SetServiceDescriptor(tmpServiceDescriptor);
            _vasioReceivers.emplace_back(std::move(rawReceiver));

            for (auto&& peer : _peers)
            {
                if (!IbServiceTraits<IbServiceT>::UseAsyncRegistration())
                {
                    _pendingSubscriptionAcknowledges.emplace_back(peer.get(), subscriptionInfo);
                }
                peer->Subscribe(subscriptionInfo);
            }
        }
    }
    template<class IbMessageT>
    void RegisterIbMsgSender(const std::string& networkName, const IIbServiceEndpoint* serviceId)
    {
        auto ibLink = GetLinkByName<IbMessageT>(networkName);
        auto&& serviceLinkMap = std::get<IbServiceToLinkMap<IbMessageT>>(_serviceToLinkMap);
        serviceLinkMap[serviceId->GetServiceDescriptor().GetNetworkName()] = ibLink;
    }

    template<class IbServiceT>
    inline void RegisterIbServiceImpl(const std::string& link, EndpointId /*endpointId*/, IbServiceT* service)
    {
        typename IbServiceT::IbReceiveMessagesTypes receiveMessageTypes{};
        typename IbServiceT::IbSendMessagesTypes sendMessageTypes{};

        util::tuple_tools::for_each(receiveMessageTypes,
            [this, &link, service](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            this->RegisterIbMsgReceiver<IbMessageT, IbServiceT>(link, service);
        }
        );

        util::tuple_tools::for_each(sendMessageTypes,
            [this, &link,  &service](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            auto& serviceId = dynamic_cast<IIbServiceEndpoint&>(*service);
            this->RegisterIbMsgSender<IbMessageT>(link, &serviceId);
        }
        );

        if (!IbServiceTraits<IbServiceT>::UseAsyncRegistration())
        {
            if (_pendingSubscriptionAcknowledges.empty())
            {
                _receivedAllSubscriptionAcknowledges.set_value();
            }
        }
    }

    template <class IbMessageT>
    void SendIbMessageImpl(const IIbServiceEndpoint* from, IbMessageT&& msg)
    {
        const auto& key = from->GetServiceDescriptor().GetNetworkName();

        auto& linkMap = std::get<IbServiceToLinkMap<std::decay_t<IbMessageT>>>(_serviceToLinkMap);
        if (linkMap.count(key) < 1)
        {
            throw std::runtime_error{ "VAsioConnection::SendIbMessageImpl: sending on empty link for " + key };
        }
        auto&& link = linkMap[key];
        link->DistributeLocalIbMessage(from, std::forward<IbMessageT>(msg));
    }

    template <class IbMessageT>
    void SendIbMessageToTargetImpl(const IIbServiceEndpoint* from, const std::string& targetParticipantName, IbMessageT&& msg)
    {
      const auto& key = from->GetServiceDescriptor().GetNetworkName();

      auto& linkMap = std::get<IbServiceToLinkMap<std::decay_t<IbMessageT>>>(_serviceToLinkMap);
      if (linkMap.count(key) < 1)
      {
        throw std::runtime_error{ "VAsioConnection::SendIbMessageImpl: sending on empty link for " + key };
      }
      auto&& link = linkMap[key];
      link->DispatchIbMessageToTarget(from, targetParticipantName, std::forward<IbMessageT>(msg));
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
    template<typename AcceptorT>
    void AcceptNextConnection(AcceptorT& acceptor);

    template<typename AcceptorT>
    auto GetAcceptor() const -> AcceptorT*
    {
        auto&& acceptor =  std::get<std::unique_ptr<AcceptorT>>(_acceptors);
        if (acceptor)
        {
            return acceptor.get();
        }
        return nullptr;
    }
private:
    // ----------------------------------------
    // private members
    std::shared_ptr<ib::cfg::v1::datatypes::ParticipantConfiguration> _config;
    std::string _participantName;
    ParticipantId _participantId{0};
    logging::ILogger* _logger{nullptr};

    //! \brief Virtual IB links by networkName according to IbConfig.
    util::tuple_tools::wrapped_tuple<IbLinkMap, IbMessageTypes> _ibLinks;
    //! \brief Lookup for links by name.
    util::tuple_tools::wrapped_tuple<IbServiceToLinkMap, IbMessageTypes> _serviceToLinkMap;

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
    std::tuple<
        std::unique_ptr<asio::ip::tcp::acceptor>,
        std::unique_ptr<asio::local::stream_protocol::acceptor>
    > _acceptors;

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

  // Hold mapping from hash to participantName
    std::map<uint64_t, std::string> _hashToParticipantName;
};

} // mw
} // namespace ib
