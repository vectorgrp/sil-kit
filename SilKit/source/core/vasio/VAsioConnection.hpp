/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <typeinfo>
#include <future>
#include <mutex>
#include <atomic>
#include <list>
#include <set>
#include <condition_variable>

#include "ParticipantConfiguration.hpp"

#include "tuple_tools/for_each.hpp"
#include "tuple_tools/wrapped_tuple.hpp"

#include "SilKitLink.hpp"
#include "IVAsioPeer.hpp"
#include "VAsioReceiver.hpp"
#include "VAsioTransmitter.hpp"
#include "VAsioMsgKind.hpp"
#include "IServiceEndpoint.hpp"
#include "traits/SilKitMsgTraits.hpp"
#include "traits/SilKitServiceTraits.hpp"

// private data types for unit testing support:
#include "TestDataTraits.hpp"

#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/can/string_utils.hpp"

#include "ProtocolVersion.hpp"
#include "SerializedMessage.hpp"
#include "Assert.hpp"
#include "ILoggerInternal.hpp"
#include "VAsioCapabilities.hpp"
#include "WireLinMessages.hpp"
#include "Uri.hpp"
#include "Metrics.hpp"

#include "IIoContext.hpp"
#include "IConnectionMethods.hpp"
#include "IConnectPeer.hpp"
#include "MakeAsioIoContext.hpp"
#include "ConnectKnownParticipants.hpp"
#include "RemoteConnectionManager.hpp"


namespace SilKit {
namespace Core {


class VAsioConnection
    : public IVAsioPeerListener
    , private IAcceptorListener
    , private IConnectKnownParticipantsListener
    , private IConnectionMethods
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioConnection(const VAsioConnection&) = delete;
    VAsioConnection(VAsioConnection&&) = delete;
    VAsioConnection(IParticipantInternal* participant, IMetricsManager* metricsManager,
                    SilKit::Config::ParticipantConfiguration config, std::string participantName,
                    ParticipantId participantId, Services::Orchestration::ITimeProvider* timeProvider,
                    ProtocolVersion version = CurrentProtocolVersion());
    ~VAsioConnection() override;

public:
    // ----------------------------------------
    // Operator Implementations
    VAsioConnection& operator=(const VAsioConnection& other) = delete;
    VAsioConnection& operator=(VAsioConnection&& other) = delete;

public:
    // ----------------------------------------
    // Public methods
    void SetLogger(Services::Logging::ILogger* logger);
    auto GetLogger() -> SilKit::Services::Logging::ILogger*;

    void JoinSimulation(std::string registryUri);

private: // JoinSimulation Helper Functions
    void OpenParticipantAcceptors(const std::string& connectUri);
    void ConnectParticipantToRegistryAndStartIoWorker(const std::string& connectUriString);
    void WaitForRegistryHandshakeToComplete(std::chrono::milliseconds timeout);
    void ConnectToKnownParticipants();
    void WaitForAllReplies(std::chrono::milliseconds timeout);

public:
    template <class SilKitServiceT>
    void RegisterSilKitService(SilKitServiceT* service)
    {
        std::future<void> allAcked;
        if (!SilKitServiceTraits<SilKitServiceT>::UseAsyncRegistration())
        {
            SILKIT_ASSERT(_pendingSubscriptionAcknowledges.empty());
            _receivedAllSubscriptionAcknowledges = std::promise<void>{};
            allAcked = _receivedAllSubscriptionAcknowledges.get_future();
        }
        else
        {
            _hasPendingAsyncSubscriptions = true;
        }

        _ioContext->Post([this, service]() { this->RegisterSilKitServiceImpl<SilKitServiceT>(service); });

        if (!SilKitServiceTraits<SilKitServiceT>::UseAsyncRegistration())
        {
            Trace(_logger, "SIL Kit waiting for subscription acknowledges for SilKitService {}.",
                  typeid(*service).name());
            allAcked.wait();
            Trace(_logger, "SIL Kit received all subscription acknowledges for SilKitService {}.",
                  typeid(*service).name());
        }
    }

    template <class SilKitServiceT>
    void SetHistoryLengthForLink(size_t historyLength, SilKitServiceT* service)
    {
        typename SilKitServiceT::SilKitSendMessagesTypes sendMessageTypes{};

        auto&& networkName = GetServiceDescriptor(service).GetNetworkName();

        Util::tuple_tools::for_each(sendMessageTypes, [this, networkName, historyLength](auto&& message) {
            using SilKitMessageT = std::decay_t<decltype(message)>;
            auto link = this->GetLinkByName<SilKitMessageT>(networkName);
            link->SetHistoryLength(historyLength);
        });
    }

    template <typename SilKitMessageT>
    void SendMsg(const IServiceEndpoint* from, SilKitMessageT&& msg)
    {
        ExecuteOnIoThread(&VAsioConnection::SendMsgImpl<SilKitMessageT>, from, std::forward<SilKitMessageT>(msg));
    }

    template <typename SilKitMessageT>
    void SendMsg(const IServiceEndpoint* from, const std::string& targetParticipantName, SilKitMessageT&& msg)
    {
        ExecuteOnIoThread(&VAsioConnection::SendMsgToTargetImpl<SilKitMessageT>, from, targetParticipantName,
                          std::forward<SilKitMessageT>(msg));
    }

    inline void OnAllMessagesDelivered(const std::function<void()>& callback)
    {
        callback();
    }

    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> function)
    {
        _ioContext->Post(std::move(function));
    }

    inline auto Config() const -> const SilKit::Config::ParticipantConfiguration&
    {
        return _config;
    }

    // Temporary Helpers
    void RegisterMessageReceiver(std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)> callback);

    // Prepare Acceptor Sockets (Local Domain and TCP)
    auto PrepareAcceptorEndpointUris(const std::string& connectUri) -> std::vector<std::string>;
    void OpenTcpAcceptors(const std::vector<std::string>& acceptorEndpointUris);
    void OpenLocalAcceptors(const std::vector<std::string>& acceptorEndpointUris);

    // Listening Sockets (acceptors)
    void AcceptLocalConnections(const std::string& uniqueId);
    auto AcceptTcpConnectionsOn(const std::string& hostname, uint16_t port) -> std::pair<std::string, uint16_t>;

    void StartIoWorker();

    void RegisterPeerShutdownCallback(std::function<void(IVAsioPeer* peer)> callback);

    void NotifyShutdown();

    void EnableAggregation();

    // Register handlers for completion of async service creation
    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler);

    size_t GetNumberOfConnectedParticipants()
    {
        return _peers.size();
    };

    auto GetNumberOfRemoteReceivers(const IServiceEndpoint* service, const std::string& msgTypeName) -> size_t;
    auto GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* service,
                                              const std::string& msgTypeName) -> std::vector<std::string>;

    bool ParticipantHasCapability(const std::string& participantName, const std::string& capability) const;

public: // IVAsioPeerListener
    void OnSocketData(IVAsioPeer* from, SerializedMessage&& buffer) override;
    void OnPeerShutdown(IVAsioPeer* peer) override;

private: // data types
    template <class MsgT>
    using SilKitLinkMap = std::map<std::string, std::shared_ptr<SilKitLink<MsgT>>>;

    template <class MsgT>
    using SilKitServiceToLinkMap = std::map<std::string, std::shared_ptr<SilKitLink<MsgT>>>;

    using ParticipantAnnouncementReceiver = std::function<void(IVAsioPeer* peer, ParticipantAnnouncement)>;

    using SilKitMessageTypes = std::tuple<
        Services::Logging::LogMsg, Services::Orchestration::NextSimTask, Services::Orchestration::SystemCommand,
        Services::Orchestration::ParticipantStatus, Services::Orchestration::WorkflowConfiguration,
        Services::PubSub::WireDataMessageEvent, Services::Rpc::FunctionCall, Services::Rpc::FunctionCallResponse,
        Services::Can::WireCanFrameEvent, Services::Can::CanFrameTransmitEvent, Services::Can::CanControllerStatus,
        Services::Can::CanConfigureBaudrate, Services::Can::CanSetControllerMode,
        Services::Ethernet::WireEthernetFrameEvent, Services::Ethernet::EthernetFrameTransmitEvent,
        Services::Ethernet::EthernetStatus, Services::Ethernet::EthernetSetMode, Services::Lin::LinSendFrameRequest,
        Services::Lin::LinSendFrameHeaderRequest, Services::Lin::LinTransmission, Services::Lin::LinWakeupPulse,
        Services::Lin::WireLinControllerConfig, Services::Lin::LinControllerStatusUpdate,
        Services::Lin::LinFrameResponseUpdate, Services::Flexray::WireFlexrayFrameEvent,
        Services::Flexray::WireFlexrayFrameTransmitEvent, Services::Flexray::FlexraySymbolEvent,
        Services::Flexray::FlexraySymbolTransmitEvent, Services::Flexray::FlexrayCycleStartEvent,
        Services::Flexray::FlexrayHostCommand, Services::Flexray::FlexrayControllerConfig,
        Services::Flexray::FlexrayTxBufferConfigUpdate, Services::Flexray::WireFlexrayTxBufferUpdate,
        Services::Flexray::FlexrayPocStatusEvent, Core::Discovery::ParticipantDiscoveryEvent,
        Core::Discovery::ServiceDiscoveryEvent, Core::RequestReply::RequestReplyCall,
        Core::RequestReply::RequestReplyCallReturn, VSilKit::MetricsUpdate,

        // Private testing data types
        Core::Tests::Version1::TestMessage, Core::Tests::Version2::TestMessage, Core::Tests::TestFrameEvent>;

private:
    // ----------------------------------------
    // private methods
    void ReceiveRawSilKitMessage(IVAsioPeer* from, SerializedMessage&& buffer);
    void ReceiveSubscriptionAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer);
    void ReceiveSubscriptionAcknowledge(IVAsioPeer* from, SerializedMessage&& buffer);
    void ReceiveRegistryMessage(IVAsioPeer* from, SerializedMessage&& buffer);
    void ReceiveProxyMessage(IVAsioPeer* from, SerializedMessage&& buffer);

    bool TryAddRemoteSubscriber(IVAsioPeer* from, const VAsioMsgSubscriber& subscriber);

    // Registry related send / receive methods
    void SendParticipantAnnouncement(IVAsioPeer* peer);
    void ReceiveParticipantAnnouncement(IVAsioPeer* from, SerializedMessage&& buffer);

    void SendParticipantAnnouncementReply(IVAsioPeer* peer);
    void SendFailedParticipantAnnouncementReply(IVAsioPeer* peer, ProtocolVersion version, std::string message);
    void ReceiveParticipantAnnouncementReply(IVAsioPeer* from, SerializedMessage&& buffer);

    void ReceiveKnownParticpants(IVAsioPeer* peer, SerializedMessage&& buffer);

    void ReceiveRemoteParticipantConnectRequest(IVAsioPeer* peer, SerializedMessage&& buffer);
    void ReceiveRemoteParticipantConnectRequest_Registry(IVAsioPeer* peer, RemoteParticipantConnectRequest msg);
    void ReceiveRemoteParticipantConnectRequest_Participant(IVAsioPeer* peer, RemoteParticipantConnectRequest msg);

    void LogAndPrintNetworkIncompatibility(const RegistryMsgHeader& other, const std::string& otherParticipantName);

    void AssociateParticipantNameAndPeer(const std::string& simulationName, const std::string& participantName,
                                         IVAsioPeer* peer);
    auto FindPeerByName(const std::string& simulationName, const std::string& participantName) const -> IVAsioPeer*;

    // Subscriptions completed Helper
    void SyncSubscriptionsCompleted();
    void AsyncSubscriptionsCompleted();
    // Unique identifier of SubscriptionAcknowledges on the subscriber
    using PendingAcksIdentifier = std::pair<IVAsioPeer*, VAsioMsgSubscriber>;
    void RemovePendingSubscription(const PendingAcksIdentifier& ackId);

    void SendProxyPeerShutdownNotification(IVAsioPeer* peer);
    void RemovePeerFromLinks(IVAsioPeer* peer);
    void RemovePeerFromConnection(IVAsioPeer* peer);

    template <class SilKitMessageT>
    auto GetLinkByName(const std::string& networkName) -> std::shared_ptr<SilKitLink<SilKitMessageT>>
    {
        std::unique_lock<decltype(_linksMx)> lock{_linksMx};

        auto& link = std::get<SilKitLinkMap<SilKitMessageT>>(_links)[networkName];
        if (!link)
        {
            link = std::make_shared<SilKitLink<SilKitMessageT>>(networkName, _logger, _timeProvider);
        }
        return link;
    }

    template <class SilKitMessageT, class SilKitServiceT>
    void RegisterSilKitMsgReceiver(IMessageReceiver<SilKitMessageT>* receiver)
    {
        SILKIT_ASSERT(_logger);
        auto&& serviceDescriptor = GetServiceDescriptor(receiver);
        auto&& networkName = serviceDescriptor.GetNetworkName();

        auto link = GetLinkByName<SilKitMessageT>(networkName);
        link->AddLocalReceiver(receiver);

        std::string msgSerdesName = SilKitMsgTraits<SilKitMessageT>::SerdesName();
        const std::string uniqueReceiverId = networkName + "/" + msgSerdesName;
        bool isNewReceiver = _vasioUniqueReceiverIds.insert(uniqueReceiverId).second;
        if (isNewReceiver)
        {
            // we have to subscribe to messages from other peers
            VAsioMsgSubscriber subscriptionInfo;
            subscriptionInfo.receiverIdx = static_cast<decltype(subscriptionInfo.receiverIdx)>(_vasioReceivers.size());
            subscriptionInfo.networkName = networkName;
            subscriptionInfo.msgTypeName = msgSerdesName;
            subscriptionInfo.version = SilKitMsgTraits<SilKitMessageT>::Version();

            std::unique_ptr<IVAsioReceiver> rawReceiver =
                std::make_unique<VAsioReceiver<SilKitMessageT>>(subscriptionInfo, link, _logger);
            auto* serviceEndpointPtr = dynamic_cast<IServiceEndpoint*>(rawReceiver.get());
            ServiceDescriptor tmpServiceDescriptor(GetServiceDescriptor(receiver));
            tmpServiceDescriptor.SetParticipantNameAndComputeId(_participantName);
            // copy the Service Endpoint Id
            serviceEndpointPtr->SetServiceDescriptor(tmpServiceDescriptor);
            _vasioReceivers.emplace_back(std::move(rawReceiver));

            {
                std::unique_lock<decltype(_peersLock)> lock{_peersLock};

                for (auto&& peer : _peers)
                {
                    // Add pending subscriptions
                    PendingAcksIdentifier ackPair{peer.get(), subscriptionInfo};
                    if (!SilKitServiceTraits<SilKitServiceT>::UseAsyncRegistration())
                    {
                        _pendingSubscriptionAcknowledges.emplace_back(ackPair);
                    }
                    else
                    {
                        _pendingAsyncSubscriptionAcknowledges.emplace_back(ackPair);
                    }

                    peer->Subscribe(subscriptionInfo);
                }
            }
        }
    }

    template <class SilKitMessageT>
    void RegisterSilKitMsgSender(const std::string& networkName)
    {
        auto link = GetLinkByName<SilKitMessageT>(networkName);
        auto&& serviceLinkMap = std::get<SilKitServiceToLinkMap<SilKitMessageT>>(_serviceToLinkMap);
        serviceLinkMap[networkName] = link;
    }

    template <class SilKitServiceT>
    inline void RegisterSilKitServiceImpl(SilKitServiceT* service)
    {
        typename SilKitServiceT::SilKitReceiveMessagesTypes receiveMessageTypes{};
        typename SilKitServiceT::SilKitSendMessagesTypes sendMessageTypes{};

        Util::tuple_tools::for_each(receiveMessageTypes, [this, service](auto&& message) {
            using SilKitMessageT = std::decay_t<decltype(message)>;
            this->RegisterSilKitMsgReceiver<SilKitMessageT, SilKitServiceT>(service);
        });

        Util::tuple_tools::for_each(sendMessageTypes, [this, service](auto&& message) {
            using SilKitMessageT = std::decay_t<decltype(message)>;
            this->RegisterSilKitMsgSender<SilKitMessageT>(GetServiceDescriptor(service).GetNetworkName());
        });

        // We could have registered a receiver that only uses already acknowledged senders, thus no new handshake is
        // triggered. In that case, the pending acks might be already empty and the subscription is completed.
        if (!SilKitServiceTraits<SilKitServiceT>::UseAsyncRegistration())
        {
            if (_pendingSubscriptionAcknowledges.empty())
            {
                SyncSubscriptionsCompleted();
            }
        }
        else
        {
            if (_pendingAsyncSubscriptionAcknowledges.empty())
            {
                AsyncSubscriptionsCompleted();
            }
        }
    }

    template <class SilKitMessageT>
    void SendMsgImpl(const IServiceEndpoint* from, SilKitMessageT&& msg)
    {
        const auto& key = from->GetServiceDescriptor().GetNetworkName();

        auto& linkMap = std::get<SilKitServiceToLinkMap<std::decay_t<SilKitMessageT>>>(_serviceToLinkMap);
        if (linkMap.count(key) < 1)
        {
            throw SilKitError{"SendMsgImpl: sending on empty link for " + key};
        }
        auto&& link = linkMap[key];
        link->DistributeLocalSilKitMessage(from, std::forward<SilKitMessageT>(msg));
    }

    template <class SilKitMessageT>
    void SendMsgToTargetImpl(const IServiceEndpoint* from, const std::string& targetParticipantName,
                             SilKitMessageT&& msg)
    {
        const auto& key = from->GetServiceDescriptor().GetNetworkName();

        auto& linkMap = std::get<SilKitServiceToLinkMap<std::decay_t<SilKitMessageT>>>(_serviceToLinkMap);
        if (linkMap.count(key) < 1)
        {
            throw SilKitError{"SendMsgToTargetImpl: sending on empty link for " + key};
        }
        auto&& link = linkMap[key];
        link->DispatchSilKitMessageToTarget(from, targetParticipantName, std::forward<SilKitMessageT>(msg));
    }

    template <typename... MethodArgs, typename... Args>
    inline void ExecuteOnIoThread(void (VAsioConnection::*method)(MethodArgs...), Args&&... args)
    {
        _ioContext->Post([=]() mutable { (this->*method)(std::move(args)...); });
    }
    inline void ExecuteOnIoThread(std::function<void()> function)
    {
        _ioContext->Post(std::move(function));
    }

    template <class SilKitServiceT>
    const ServiceDescriptor& GetServiceDescriptor(SilKitServiceT* service)
    {
        return dynamic_cast<IServiceEndpoint&>(*service).GetServiceDescriptor();
    }

public:
    auto GetIoContext() -> IIoContext*;

private:
    auto MakePeerInfo() -> VAsioPeerInfo;

private: // IConnectionMethods
    auto MakeConnectPeer(const VAsioPeerInfo& peerInfo) -> std::unique_ptr<IConnectPeer> override;
    auto MakeVAsioPeer(std::unique_ptr<IRawByteStream> stream) -> std::unique_ptr<IVAsioPeer> override;

    void HandleConnectedPeer(IVAsioPeer* peer) override;
    void AddPeer(std::unique_ptr<IVAsioPeer> peer) override;

    auto TryRemoteConnectRequest(const VAsioPeerInfo& peerInfo) -> bool override;
    auto TryProxyConnect(const VAsioPeerInfo& peerInfo) -> bool override;

private: // IAcceptorListener
    void OnAsyncAcceptSuccess(IAcceptor& acceptor, std::unique_ptr<IRawByteStream> stream) override;
    void OnAsyncAcceptFailure(IAcceptor& acceptor) override;

private: // IConnectionManagerListener
    void OnConnectKnownParticipantsFailure(ConnectKnownParticipants&) override;
    void OnConnectKnownParticipantsWaitingForAllReplies(ConnectKnownParticipants&) override;
    void OnConnectKnownParticipantsAllRepliesReceived(ConnectKnownParticipants&) override;

private:
    void OnRemoteConnectionSuccess(std::unique_ptr<SilKit::Core::IVAsioPeer> vAsioPeer);
    void OnRemoteConnectionFailure(VAsioPeerInfo peerInfo);

private:
    // ----------------------------------------
    // private members
    SilKit::Config::ParticipantConfiguration _config;
    std::string _participantName;
    ParticipantId _participantId{0};
    Services::Logging::ILogger* _logger{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};

    std::string _simulationName;
    bool _allowAnySimulationName{true};

    mutable std::mutex _linksMx;

    //! \brief Virtual SIL Kit links by networkName according to SilKitConfig.
    Util::tuple_tools::wrapped_tuple<SilKitLinkMap, SilKitMessageTypes> _links;
    //! \brief Lookup for links by name.
    Util::tuple_tools::wrapped_tuple<SilKitServiceToLinkMap, SilKitMessageTypes> _serviceToLinkMap;

    std::vector<std::unique_ptr<IVAsioReceiver>> _vasioReceivers;
    std::unordered_set<std::string> _vasioUniqueReceiverIds;

    std::mutex _participantAnnouncementReceiversMutex;
    std::vector<ParticipantAnnouncementReceiver> _participantAnnouncementReceivers;
    std::vector<std::function<void(IVAsioPeer*)>> _peerShutdownCallbacks;

    VAsioCapabilities _capabilities;

    //! protects access to _registry and _peers
    std::mutex _peersLock;

    std::unique_ptr<IIoContext> _ioContext;

    std::unique_ptr<IVAsioPeer> _registry{nullptr};
    std::vector<std::unique_ptr<IVAsioPeer>> _peers;

    std::mutex _acceptorsMutex;
    std::vector<std::unique_ptr<IAcceptor>> _acceptors;

    ConnectKnownParticipants _connectKnownParticipants;
    RemoteConnectionManager _remoteConnectionManager;

    /// The promise is set when the registry connection handshake has been completed. An exception is set, if the
    /// handshake failed.
    std::promise<void> _registryHandshakeComplete;
    /// The promise is set when all known participant connections are waiting for the handshake to complete. An
    /// exception is set, if any connection attempt failed completely.
    std::promise<void> _startWaitingForParticipantHandshakes;
    /// The promise is set when all handshakes with all known participants have completed. An exception is set, if any
    /// connection attempt failed completely.
    std::promise<void> _allKnownParticipantHandshakesComplete;

    /// Protects access to _participantNameToPeer
    mutable std::mutex _mutex;

    // Keep track of the sent Subscriptions when Registering an SIL Kit Service
    std::vector<PendingAcksIdentifier> _pendingSubscriptionAcknowledges;
    std::promise<void> _receivedAllSubscriptionAcknowledges;

    // Subscriptions for internal services that use async registration
    std::vector<PendingAcksIdentifier> _pendingAsyncSubscriptionAcknowledges;
    Util::SynchronizedHandlers<std::function<void()>> _asyncSubscriptionsCompletionHandlers;
    std::atomic<bool> _hasPendingAsyncSubscriptions{false};

    // The worker thread should be the last members in this class. This ensures
    // that no callback is destroyed before the thread finishes.
    std::thread _ioWorker;

    //We violate the strict layering architecture, so that we can cleanly shutdown without false error messages.
    std::atomic_bool _isShuttingDown{false};

    // Hold mapping from simulationName to mapping from participantName to peer
    std::unordered_map<std::string, std::unordered_map<std::string, IVAsioPeer*>> _participantNameToPeer;

    // Hold mapping from proxy source to all proxy destinations (used by registry for shutdown information)
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<std::string>>>
        _proxySourceToDestinations;

    // Hold mapping from proxied peer to all proxy peers being served via the key.
    std::unordered_map<IVAsioPeer*, std::unordered_set<IVAsioPeer*>> _peerToProxyPeers;

    // unit testing support
    ProtocolVersion _version;
    friend class Test_VAsioConnection;

    // metrics
    IMetricsManager* _metricsManager;

    // for debugging purposes:
    IParticipantInternal* _participant{nullptr};

    friend class ::SilKit::Core::RemoteConnectionManager;

    bool _useAggregation{false};
};


} // namespace Core
} // namespace SilKit
