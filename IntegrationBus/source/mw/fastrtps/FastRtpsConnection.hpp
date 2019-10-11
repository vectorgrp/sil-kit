// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <future>

#include "ib/cfg/Config.hpp"

#include "fastrtps_fwd.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

#include "idl/all.hpp"

#include "IdlTraits.hpp"
#include "IdlTypeConversion.hpp"
#include "IdlTypeConversionLogging_impl.hpp"

#include "IbSubListener.hpp"
#include "ReportMatchingListener.hpp"
#include "FastRtpsUtils.hpp"

#include "tuple_tools/for_each.hpp"

namespace ib {
namespace mw {

class FastRtpsConnection
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FastRtpsConnection() = default;
    FastRtpsConnection(const FastRtpsConnection&) = default;
    FastRtpsConnection(FastRtpsConnection&&) = default;
    FastRtpsConnection(cfg::Config config, std::string participantName, ParticipantId participantId);

public:
    // ----------------------------------------
    // Operator Implementations
    FastRtpsConnection& operator=(FastRtpsConnection& other) = default;
    FastRtpsConnection& operator=(FastRtpsConnection&& other) = default;

public:
    // ----------------------------------------
    // Public methods
    //
    void JoinDomain(uint32_t domainId);

    template<class IbServiceT>
    inline void RegisterIbService(const std::string& topicName, EndpointId endpointId, IbServiceT* receiver);

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

    void OnAllMessagesDelivered(std::function<void(void)> callback);
    void FlushSendBuffers();

private:
    // ----------------------------------------
    // private datatypes
    struct RtpsPubListener
    {
        std::unique_ptr<eprosima::fastrtps::PublisherListener> listener;
        std::unique_ptr<eprosima::fastrtps::Publisher, FastRtps::RemovePublisher> publisher;
    };

    template<class TopicT>
    struct RtpsSubListener
    {
        IbSubListener<TopicT> listener;
        std::unique_ptr<eprosima::fastrtps::Subscriber, FastRtps::RemoveSubscriber> subscriber;
    };

    template<class TopicT>
    struct RtpsTopics
    {
        using TopicType = TopicT;
        using PubSubType = typename TopicTrait<TopicT>::PubSubType;

        PubSubType pubSubType;
        std::unordered_map<std::string, RtpsPubListener> pubListeners;
        std::unordered_map<std::string, RtpsSubListener<TopicT>> subListeners;
        std::unordered_map<EndpointId, eprosima::fastrtps::Publisher*> endpointToPublisherMap;
    };

    template<typename ControllerT>
    using ControllerMap = std::unordered_map<EndpointId, std::unique_ptr<ControllerT>>;

private:
    // ----------------------------------------
    // private methods
    void registerTopicTypeIfNecessary(eprosima::fastrtps::TopicDataType* topicType);
    template <class AttrT>
    void SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType);
    auto MakeFastrtpsProfileName(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType) -> std::string;
    auto createPublisher(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType, eprosima::fastrtps::PublisherListener* listener = nullptr) -> eprosima::fastrtps::Publisher*;
    auto createSubscriber(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType, eprosima::fastrtps::SubscriberListener* listener = nullptr) -> eprosima::fastrtps::Subscriber*;

    // --------------------------------------------------------------------------------
    // Manage RtpsTopics, which supports multiple topics with the same topic type.
    // --------------------------------------------------------------------------------
    template <class IbMessageT>
    void PublishRtpsTopic(const std::string& topicName, EndpointId endpointId);
    template <class IControllerT>
    void PublishRtpsTopics(const std::string& topicName, EndpointId endpointId);

    template <class IbMessageT>
    void SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<IbMessageT>* receiver);
    template<class EndpointT>
    void SubscribeRtpsTopics(const std::string& topicName, EndpointId endpointId, EndpointT* receiver);

private:
    // ----------------------------------------
    // private members
    cfg::Config _config;
    std::string _participantName;
    ParticipantId _participantId{0};

    std::unique_ptr<eprosima::fastrtps::Participant, FastRtps::RemoveParticipant> _fastRtpsParticipant;

    std::tuple<
        RtpsTopics<sim::can::idl::CanMessage>,
        RtpsTopics<sim::can::idl::CanTransmitAcknowledge>,
        RtpsTopics<sim::can::idl::CanControllerStatus>,
        RtpsTopics<sim::can::idl::CanConfigureBaudrate>,
        RtpsTopics<sim::can::idl::CanSetControllerMode>,
        RtpsTopics<sim::eth::idl::EthMessage>,
        RtpsTopics<sim::eth::idl::EthTransmitAcknowledge>,
        RtpsTopics<sim::eth::idl::EthStatus>,
        RtpsTopics<sim::eth::idl::EthSetMode>,
        RtpsTopics<sim::fr::idl::FrMessage>,
        RtpsTopics<sim::fr::idl::FrMessageAck>,
        RtpsTopics<sim::fr::idl::FrSymbol>,
        RtpsTopics<sim::fr::idl::FrSymbolAck>,
        RtpsTopics<sim::fr::idl::CycleStart>,
        RtpsTopics<sim::fr::idl::HostCommand>,
        RtpsTopics<sim::fr::idl::ControllerConfig>,
        RtpsTopics<sim::fr::idl::TxBufferConfigUpdate>,
        RtpsTopics<sim::fr::idl::TxBufferUpdate>,
        RtpsTopics<sim::fr::idl::ControllerStatus>,
        RtpsTopics<sim::lin::idl::SendFrameRequest>,
        RtpsTopics<sim::lin::idl::SendFrameHeaderRequest>,
        RtpsTopics<sim::lin::idl::Transmission>,
        RtpsTopics<sim::lin::idl::WakeupPulse>,
        RtpsTopics<sim::lin::idl::ControllerConfig>,
        RtpsTopics<sim::lin::idl::ControllerStatusUpdate>,
        RtpsTopics<sim::lin::idl::FrameResponseUpdate>,
        RtpsTopics<sim::generic::idl::GenericMessage>,
        RtpsTopics<sim::io::idl::AnalogIoMessage>,
        RtpsTopics<sim::io::idl::DigitalIoMessage>,
        RtpsTopics<sim::io::idl::PatternIoMessage>,
        RtpsTopics<sim::io::idl::PwmIoMessage>,
        RtpsTopics<sync::idl::ParticipantCommand>,
        RtpsTopics<sync::idl::SystemCommand>,
        RtpsTopics<sync::idl::NextSimTask>,
        RtpsTopics<sync::idl::Tick>,
        RtpsTopics<sync::idl::TickDone>,
        RtpsTopics<sync::idl::QuantumGrant>,
        RtpsTopics<sync::idl::QuantumRequest>,
        RtpsTopics<sync::idl::ParticipantStatus>,
        RtpsTopics<logging::idl::LogMsg>
    > _rtpsTopics;

    std::vector<eprosima::fastrtps::Publisher*> _allPublishers;

    std::future<void> _messagesDelivered;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IbServiceT>
void FastRtpsConnection::RegisterIbService(const std::string& topicName, EndpointId endpointId, IbServiceT* receiver)
{
    PublishRtpsTopics<IbServiceT>(topicName, endpointId);
    SubscribeRtpsTopics(topicName, endpointId, receiver);
}

template<typename IbMessageT>
void FastRtpsConnection::SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
{
    auto idlMsg = to_idl(std::forward<IbMessageT>(msg));
    idlMsg.senderAddr(to_idl(from));

    auto& rtpsTopics = std::get<RtpsTopics<decltype(idlMsg)>>(_rtpsTopics);
    assert(rtpsTopics.endpointToPublisherMap.find(from.endpoint) != rtpsTopics.endpointToPublisherMap.end());

    auto* publisher = rtpsTopics.endpointToPublisherMap[from.endpoint];
    publisher->write(&idlMsg);
}

template <class AttrT>
void FastRtpsConnection::SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType)
{
    using namespace eprosima::fastrtps;
    using namespace eprosima::fastrtps::rtps;

    attributes.topic.topicKind = (topicType->m_isGetKeyDefined)
        ? WITH_KEY
        : NO_KEY;
    attributes.topic.topicDataType = topicType->getName();
    attributes.topic.topicName = topicName.c_str();
    attributes.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    attributes.topic.historyQos.depth = 5;
    attributes.topic.resourceLimitsQos.max_instances = 64;
    attributes.topic.resourceLimitsQos.max_samples_per_instance = 5;
    attributes.topic.resourceLimitsQos.max_samples = 320;  // Constraint when m_isGetKeyDefined == WITH_KEY: max_samples >= max_samples_per_instance*max_instances
    attributes.topic.resourceLimitsQos.allocated_samples = 40;
    attributes.historyMemoryPolicy = PREALLOCATED_WITH_REALLOC_MEMORY_MODE;  // https://github.com/eProsima/Fast-RTPS/issues/74
    attributes.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
}

template<class IbMessageT>
void FastRtpsConnection::PublishRtpsTopic(const std::string& topicName, EndpointId endpointId)
{
    using IdlMessageT = to_idl_message_t<IbMessageT>;
    auto&& rtpsTopics = std::get<RtpsTopics<IdlMessageT>>(_rtpsTopics);

    if (rtpsTopics.pubListeners.find(topicName) == rtpsTopics.pubListeners.end())
    {
        auto&& rtpsPublisher = rtpsTopics.pubListeners[topicName];
        rtpsPublisher.listener = std::make_unique<PubMatchedListener>();
        rtpsPublisher.publisher.reset(createPublisher(topicName, &rtpsTopics.pubSubType, rtpsPublisher.listener.get()));
    }
    rtpsTopics.endpointToPublisherMap[endpointId] = rtpsTopics.pubListeners[topicName].publisher.get();
}

template <class IbMessageT>
void FastRtpsConnection::SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<IbMessageT>* receiver)
{
    using IdlMessageT = to_idl_message_t<IbMessageT>;
    auto&& rtpsTopics = std::get<RtpsTopics<IdlMessageT>>(_rtpsTopics);

    if (rtpsTopics.subListeners.find(topicName) == rtpsTopics.subListeners.end())
    {
        // create a subscriber entry
        auto&& rtpsSubscriber = rtpsTopics.subListeners[topicName];
        rtpsSubscriber.subscriber.reset(createSubscriber(topicName, &rtpsTopics.pubSubType, &rtpsSubscriber.listener));
    }
    rtpsTopics.subListeners[topicName].listener.addReceiver(receiver);
}

template<class IbSenderT>
void FastRtpsConnection::PublishRtpsTopics(const std::string& topicName, EndpointId endpointId)
{
    typename IbSenderT::IbSendMessagesTypes sendMessageTypes{};

    util::tuple_tools::for_each(sendMessageTypes,
        [this, &topicName, &endpointId](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            this->PublishRtpsTopic<IbMessageT>(topicName, endpointId);
        }
    );
}

template<class EndpointT>
void FastRtpsConnection::SubscribeRtpsTopics(const std::string& topicName, EndpointId /*endpointId*/, EndpointT* receiver)
{
    typename EndpointT::IbReceiveMessagesTypes receiveMessageTypes{};

    util::tuple_tools::for_each(receiveMessageTypes,
        [this, &topicName, receiver](auto&& ibMessage)
        {
            using IbMessageT = std::decay_t<decltype(ibMessage)>;
            this->SubscribeRtpsTopic<IbMessageT>(topicName, receiver);
        }
    );
}


} // mw
} // namespace ib
