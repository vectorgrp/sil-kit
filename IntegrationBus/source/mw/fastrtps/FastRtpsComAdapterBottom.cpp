// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FastRtpsComAdapterBottom.hpp"

#include <cassert>
#include <sstream>

#include "fastrtps/Domain.h"
#include "fastrtps/utils/IPLocator.h"

#include "IdlTraits.hpp"
#include "IdlTypeConversionLogging_impl.hpp"

#include "ReportMatchingListener.hpp"

#include "tuple_tools/bind.hpp"
#include "tuple_tools/for_each.hpp"
#include "tuple_tools/predicative_get.hpp"

#include "ib/cfg/string_utils.hpp"


namespace ib {
namespace mw {

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using namespace ib::sim;

namespace tt = util::tuple_tools;

template<typename TopicT>
using PubSubType = typename TopicTrait<TopicT>::PubSubType;


void FastRtpsComAdapterBottom::joinIbDomain(uint32_t domainId)
{
    joinDomain(domainId);
}

void FastRtpsComAdapterBottom::joinDomain(uint32_t domainId)
{
    if (_fastRtpsParticipant)
        throw std::exception();

    //CREATE RTPSParticipant

    Participant* participant = nullptr;

    auto&& fastRtpsCfg = _config.middlewareConfig.fastRtps;

    if (fastRtpsCfg.discoveryType == cfg::FastRtps::DiscoveryType::ConfigFile)
    {
        // Create participant based on profile specified in file
        auto configFilePath = _config.configPath + fastRtpsCfg.configFileName;
        if (Domain::loadXMLProfilesFile(configFilePath))
        {
            participant = Domain::createParticipant(_participantName);
        }
    }
    else
    {
        // Create participant based on standard profile
        ParticipantAttributes pParam;

        //pParam.rtps.defaultSendPort = 10042;
        pParam.rtps.participantID = _participantId;
        pParam.rtps.setName(_participantName.c_str());
        pParam.rtps.builtin.domainId = domainId;
        pParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
        pParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
        pParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
        pParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
        pParam.rtps.builtin.leaseDuration = c_TimeInfinite;

        auto CalculateMetaTrafficPort = [domainId, &portParams = pParam.rtps.port](auto participantId)
        {
            // cf. http://docs.eprosima.com/en/latest/pubsub.html#id6
            // Metatraffic unicast	PB + DG * domainId + offsetd1 + PG * participantId
            // DG: DomainId Gain. You can set this value using attribute rtps.port.domainIDGain. Default value is 250.
            // PG : ParticipantId Gain. You can set this value using attribute rtps.port.participantIDGain. Default value is 2.
            // PB : Port Base number. You can set this value using attribute rtps.port.portBase. Default value is 7400.
            // offsetd0, offsetd1, offsetd2, offsetd3 : Additional offsets. You can set these values using attributes rtps.port.offsetdN. Default values are : offsetd0 = 0, offsetd1 = 10, offsetd2 = 1, offsetd3 = 11.
            return portParams.portBase + portParams.domainIDGain * domainId + portParams.offsetd1 + portParams.participantIDGain * participantId;
            //return portParams.portBase + portParams.domainIDGain * domainId + portParams.participantIDGain * participantId;
        };

        std::cout << "FastRtpsComAdapterBottom is using DiscoverType: " << fastRtpsCfg.discoveryType << "\n";
        switch (fastRtpsCfg.discoveryType)
        {
        case cfg::FastRtps::DiscoveryType::Local:
        {
            Locator_t unicastLocator;
            pParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(unicastLocator);

            for (auto&& otherParticipant : _config.simulationSetup.participants)
            {
                if (otherParticipant.id == _participantId)
                    continue;

                Locator_t participantLocator;
                auto port = CalculateMetaTrafficPort(otherParticipant.id);
                IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", port, participantLocator);
                pParam.rtps.builtin.initialPeersList.push_back(participantLocator);
            }
            break;
        }

        case cfg::FastRtps::DiscoveryType::Unicast:
        {
            Locator_t unicastLocator;
            pParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(unicastLocator);

            for (auto&& otherParticipant : _config.simulationSetup.participants)
            {
                if (otherParticipant.id == _participantId)
                    continue;

                Locator_t participantLocator;
                auto port = CalculateMetaTrafficPort(otherParticipant.id);
                try
                {
                    auto&& participantIp = fastRtpsCfg.unicastLocators.at(otherParticipant.name);
                    IPLocator::createLocator(LOCATOR_KIND_UDPv4, participantIp, port, participantLocator);
                }
                catch (const std::out_of_range&)
                {
                    throw cfg::Misconfiguration{"No UnicastLocator configured for participant \"" + otherParticipant.name + "\""};
                }
                std::cout << "Adding initial peer for unicast discovery=" << IPLocator::toIPv4string(participantLocator) << ":" << participantLocator.port << "\n";
                pParam.rtps.builtin.initialPeersList.push_back(participantLocator);
            }

            break;

        }

        case cfg::FastRtps::DiscoveryType::Multicast:
            // This is the FastRTPS default; so there is nothing for us to do here. Yay! \o/
            break;

        case cfg::FastRtps::DiscoveryType::ConfigFile:
            assert(false);

        default:
            throw cfg::Misconfiguration{"Invalid FastRTPS configuration"};
        }

        participant = Domain::createParticipant(pParam);
    }

    if (participant == nullptr)
        throw std::exception();

    _fastRtpsParticipant.reset(participant);
}


void FastRtpsComAdapterBottom::registerTopicTypeIfNecessary(TopicDataType* topicType)
{
    auto* participant = _fastRtpsParticipant.get();
    TopicDataType* registeredType = nullptr;
    if (!Domain::getRegisteredType(participant, topicType->getName(), &registeredType))
        Domain::registerType(participant, topicType);

    assert(Domain::getRegisteredType(participant, topicType->getName(), &registeredType));
    assert(registeredType);
}

template <class AttrT>
void FastRtpsComAdapterBottom::SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, TopicDataType* topicType)
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

auto FastRtpsComAdapterBottom::MakeFastrtpsProfileName(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType) -> std::string
{
    std::string shortTypeName{topicType->getName()};
    auto colonPos = shortTypeName.rfind(':');
    if (colonPos != std::string::npos)
    {
        shortTypeName = shortTypeName.substr(colonPos + 1);
    }
    return _participantName + '-' + shortTypeName + '-' + topicName;
}

auto FastRtpsComAdapterBottom::createPublisher(const std::string& topicName, TopicDataType* topicType, PublisherListener* listener) -> Publisher*
{
    assert(_fastRtpsParticipant);

    registerTopicTypeIfNecessary(topicType);

    Publisher* publisher{nullptr};
    if (!_config.middlewareConfig.fastRtps.configFileName.empty())
    {
        // Create publisher based on profile specified in file
        publisher = Domain::createPublisher(_fastRtpsParticipant.get(), MakeFastrtpsProfileName(topicName, topicType), listener);
    }
    else
    {
        PublisherAttributes pubAttributes;
        SetupPubSubAttributes(pubAttributes, topicName, topicType);

        // We must configure the heartbeat period when using strict SyncPolicy because subscribers
        // only send an acknowledgement in reply to a heartbeat. Thus, the hearbeat
        // period must be set to a shorter duration than the tickPeriod when using Strict
        // SyncPolicy. For Loose SyncPolicy, the FastRTPS default is sufficient.
        if (_config.simulationSetup.timeSync.syncPolicy == cfg::TimeSync::SyncPolicy::Strict)
        {
            auto tickPeriod = std::chrono::duration_cast<std::chrono::duration<long double, std::ratio<1>>>(_config.simulationSetup.timeSync.tickPeriod);
            auto heartBeatPeriod = tickPeriod / 10.0;

            pubAttributes.times.heartbeatPeriod = Time_t{heartBeatPeriod.count()};
        }
        publisher = Domain::createPublisher(_fastRtpsParticipant.get(), pubAttributes, listener);
    }

    if (publisher == nullptr)
        throw std::exception();

    _allPublishers.push_back(publisher);

    return publisher;
}

auto FastRtpsComAdapterBottom::createSubscriber(const std::string& topicName, TopicDataType* topicType, SubscriberListener* listener) -> Subscriber*
{
    assert(_fastRtpsParticipant);

    registerTopicTypeIfNecessary(topicType);

    Subscriber* subscriber;
    if (!_config.middlewareConfig.fastRtps.configFileName.empty())
    {
        // Create subscriber based on profile specified in file (already loaded with participant)
        subscriber = Domain::createSubscriber(_fastRtpsParticipant.get(), MakeFastrtpsProfileName(topicName, topicType), listener);
    }
    else
    {
        SubscriberAttributes subAttributes;
        SetupPubSubAttributes(subAttributes, topicName, topicType);
        subscriber = Domain::createSubscriber(_fastRtpsParticipant.get(), subAttributes, listener);
    }

    if (subscriber == nullptr)
        throw std::exception();

    return subscriber;
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

void FastRtpsComAdapterBottom::WaitForMessageDelivery()
{
    for (auto publisher : _allPublishers)
    {
        /* NB: you must not use c_TimeInfinite as the parameter to wait_for_all_acked()!
         *
         * FastRTPS converts the parameter to std::chrono::microseconds
         * and passes it on to std::condition_variable::wait_for().
         * However, this causes an integer overflow. Instead of
         * waiting indefinitely, it causes wait_for_all_acked() to
         * return immediately.
         */
        auto allAcked = publisher->wait_for_all_acked(Time_t{1200, 0});
        assert(allAcked);
    }
}

void FastRtpsComAdapterBottom::FlushSendBuffers()
{
    for (auto publisher : _allPublishers)
    {
        publisher->removeAllChange(nullptr);
    }
}

} // namespace mw
} // namespace ib
