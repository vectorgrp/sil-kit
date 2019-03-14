// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

//#include "ib/mw/IComAdapter.hpp"
//
#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "ib/cfg/Config.hpp"
//#include "ib/mw/all.hpp"
//#include "ib/sim/all.hpp"
//
#include "fastrtps_fwd.h"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"
//
#include "idl/all.hpp"
//
#include "IdlTypeConversion.hpp"
#include "IbSubListener.hpp"
#include "memory_fastrtps.hpp"
#include "FastRtpsGuard.hpp"


namespace ib {
namespace mw {

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

public:
    // ----------------------------------------
    // Operator Implementations
    FastRtpsComAdapterBottom& operator=(FastRtpsComAdapterBottom& other) = default;
    FastRtpsComAdapterBottom& operator=(FastRtpsComAdapterBottom&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    void WaitForMessageDelivery();
    void FlushSendBuffers();

public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    * 
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    * 
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    void joinIbDomain(uint32_t domainId);

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
    void joinDomain(uint32_t domainId);
    void OnFastrtpsDomainJoined();

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

    template <class IControllerT>
    void PublishRtpsTopics(const std::string& topicName, EndpointId endpointId);
    template<class EndpointT>
    void SubscribeRtpsTopics(const std::string& topicName, EndpointT* receiver);

    template<typename IbMessageT>
    void SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg);

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

} // mw
} // namespace ib
