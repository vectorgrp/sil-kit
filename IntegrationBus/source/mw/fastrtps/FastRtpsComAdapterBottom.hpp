// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"

#include "fastrtps_fwd.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

#include "idl/all.hpp"

#include "IdlTraits.hpp"
#include "IdlTypeConversion.hpp"
#include "IdlTypeConversionLogging_impl.hpp"

#include "IbSubListener.hpp"
#include "memory_fastrtps.hpp"
#include "FastRtpsGuard.hpp"


namespace ib {
namespace mw {


template<typename TopicT>
using PubSubType = typename TopicTrait<TopicT>::PubSubType;


class FastRtpsComAdapterBottom
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FastRtpsComAdapterBottom() = default;
    FastRtpsComAdapterBottom(const FastRtpsComAdapterBottom&) = default;
    FastRtpsComAdapterBottom(FastRtpsComAdapterBottom&&) = default;
    FastRtpsComAdapterBottom(cfg::Config config, std::string participantName);

public:
    // ----------------------------------------
    // Operator Implementations
    FastRtpsComAdapterBottom& operator=(FastRtpsComAdapterBottom& other) = default;
    FastRtpsComAdapterBottom& operator=(FastRtpsComAdapterBottom&& other) = default;

public:
    // ----------------------------------------
    // Public methods
    //
    void joinDomain(uint32_t domainId);

    template <class IControllerT>
    void PublishRtpsTopics(const std::string& topicName, EndpointId endpointId);
    template<class EndpointT>
    void SubscribeRtpsTopics(const std::string& topicName, EndpointT* receiver);

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

    void WaitForMessageDelivery();
    void FlushSendBuffers();

private:
    // ----------------------------------------
    // private datatypes
    struct RtpsPubListener
    {
        std::unique_ptr<eprosima::fastrtps::PublisherListener> listener;
        FastRtps::unique_ptr<eprosima::fastrtps::Publisher> publisher;
    };

    template<typename TopicT>
    struct RtpsSubListener
    {
        using TopicType = TopicT;
        IbSubListener<TopicType> listener;
        FastRtps::unique_ptr<eprosima::fastrtps::Subscriber> subscriber;
    };

    template<typename TopicT>
    struct RtpsTopics
    {
        using TopicType = TopicT;

        std::unique_ptr<eprosima::fastrtps::TopicDataType> pubSubType;

        std::unordered_map<std::string, RtpsPubListener> pubListeners;
        std::unordered_map<std::string, RtpsSubListener<TopicType>> subListeners;

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
    template <class IdlMessageT>
    void PublishRtpsTopic(const std::string& topicName, EndpointId endpointId);
    template <class IdlMessageT>
    void SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<to_ib_message_t<IdlMessageT>>* receiver);

private:
    // ----------------------------------------
    // private members
    FastRtps::FastRtpsGuard _fastRtpsGuard;

    cfg::Config _config;
    std::string _participantName;
    ParticipantId _participantId{0};

    FastRtps::unique_ptr<eprosima::fastrtps::Participant> _fastRtpsParticipant;

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
        RtpsTopics<sim::fr::idl::HostCommand>,
        RtpsTopics<sim::fr::idl::ControllerConfig>,
        RtpsTopics<sim::fr::idl::TxBufferUpdate>,
        RtpsTopics<sim::fr::idl::ControllerStatus>,
        RtpsTopics<sim::lin::idl::LinMessage>,
        RtpsTopics<sim::lin::idl::RxRequest>,
        RtpsTopics<sim::lin::idl::TxAcknowledge>,
        RtpsTopics<sim::lin::idl::WakeupRequest>,
        RtpsTopics<sim::lin::idl::ControllerConfig>,
        RtpsTopics<sim::lin::idl::SlaveConfiguration>,
        RtpsTopics<sim::lin::idl::SlaveResponse>,
        RtpsTopics<sim::generic::idl::GenericMessage>,
        RtpsTopics<sim::io::idl::AnalogIoMessage>,
        RtpsTopics<sim::io::idl::DigitalIoMessage>,
        RtpsTopics<sim::io::idl::PatternIoMessage>,
        RtpsTopics<sim::io::idl::PwmIoMessage>,
        RtpsTopics<sync::idl::ParticipantCommand>,
        RtpsTopics<sync::idl::SystemCommand>,
        RtpsTopics<sync::idl::Tick>,
        RtpsTopics<sync::idl::TickDone>,
        RtpsTopics<sync::idl::QuantumGrant>,
        RtpsTopics<sync::idl::QuantumRequest>,
        RtpsTopics<sync::idl::ParticipantStatus>,
        RtpsTopics<logging::idl::LogMsg>
    > _rtpsTopics;

    std::vector<eprosima::fastrtps::Publisher*> _allPublishers;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class AttrT>
void FastRtpsComAdapterBottom::SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType)
{
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

template<class IdlMessageT>
void FastRtpsComAdapterBottom::PublishRtpsTopic(const std::string& topicName, EndpointId endpointId)
{
    auto&& rtpsTopics = std::get<RtpsTopics<IdlMessageT>>(_rtpsTopics);

    if (!rtpsTopics.pubSubType)
    {
        rtpsTopics.pubSubType = std::make_unique<PubSubType<IdlMessageT>>();
    }

    if (rtpsTopics.pubListeners.find(topicName) == rtpsTopics.pubListeners.end())
    {
        auto&& rtpsPublisher = rtpsTopics.pubListeners[topicName];
        rtpsPublisher.listener = std::make_unique<PubMatchedListener>();
        rtpsPublisher.publisher.reset(createPublisher(topicName, rtpsTopics.pubSubType.get(), rtpsPublisher.listener.get()));
    }
    rtpsTopics.endpointToPublisherMap[endpointId] = rtpsTopics.pubListeners[topicName].publisher.get();
}

template <class IdlMessageT>
void FastRtpsComAdapterBottom::SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<to_ib_message_t<IdlMessageT>>* receiver)
{
    auto&& rtpsTopics = std::get<RtpsTopics<IdlMessageT>>(_rtpsTopics);

    if (!rtpsTopics.pubSubType)
    {
        rtpsTopics.pubSubType = std::make_unique<PubSubType<IdlMessageT>>();
    }

    if (rtpsTopics.subListeners.find(topicName) == rtpsTopics.subListeners.end())
    {
        // create a subscriber entry
        auto&& rtpsSubscriber = rtpsTopics.subListeners[topicName];
        rtpsSubscriber.subscriber.reset(createSubscriber(topicName, rtpsTopics.pubSubType.get(), &rtpsSubscriber.listener));
    }
    rtpsTopics.subListeners[topicName].listener.addReceiver(receiver);
}

template<class IbSenderT>
void FastRtpsComAdapterBottom::PublishRtpsTopics(const std::string& topicName, EndpointId endpointId)
{
    typename IbSenderT::IbSendMessagesTypes sendMessageTypes{};

    tt::for_each(sendMessageTypes,
        [this, &topicName, &endpointId](auto&& ibMessage)
    {
        using IbMessageT = std::decay_t<decltype(ibMessage)>;
        this->PublishRtpsTopic<to_idl_message_t<IbMessageT>>(topicName, endpointId);
    }
    );
}

template<class EndpointT>
void FastRtpsComAdapterBottom::SubscribeRtpsTopics(const std::string& topicName, EndpointT* receiver)
{
    typename EndpointT::IbReceiveMessagesTypes receiveMessageTypes{};

    tt::for_each(receiveMessageTypes,
        [this, &topicName, receiver](auto&& ibMessage)
    {
        using IbMessageT = std::decay_t<decltype(ibMessage)>;
        this->SubscribeRtpsTopic<to_idl_message_t<IbMessageT>>(topicName, receiver);
    }
    );
}

template<typename IbMessageT>
void FastRtpsComAdapterBottom::SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
{
    auto idlMsg = to_idl(std::forward<IbMessageT>(msg));
    idlMsg.senderAddr(to_idl(from));

    auto& rtpsTopics = std::get<RtpsTopics<decltype(idlMsg)>>(_rtpsTopics);
    assert(rtpsTopics.endpointToPublisherMap.find(from.endpoint) != rtpsTopics.endpointToPublisherMap.end());

    auto* publisher = rtpsTopics.endpointToPublisherMap[from.endpoint];
    publisher->write(&idlMsg);
}

} // mw
} // namespace ib
