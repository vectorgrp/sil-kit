// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "FastRtpsComAdapter.hpp"

#include <cassert>
#include <sstream>

#include "fastrtps/Domain.h"
#include "fastrtps/utils/IPLocator.h"

#include "CanController.hpp"
#include "CanControllerProxy.hpp"
#include "EthController.hpp"
#include "EthControllerProxy.hpp"
#include "FrController.hpp"
#include "FrControllerProxy.hpp"
#include "LinController.hpp"
#include "LinControllerProxy.hpp"
#include "InPort.hpp"
#include "OutPort.hpp"
#include "GenericPublisher.hpp"
#include "GenericSubscriber.hpp"
#include "ParticipantController.hpp"
#include "SystemController.hpp"
#include "SystemMonitor.hpp"
#include "SyncMaster.hpp"

#include "IdlTraits.hpp"

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


// Anonymous namespace for Helper Traits and Functions
namespace {

    template<class T, class U>
    struct IsControllerMap : std::false_type {};
    template<class T, class U>
    struct IsControllerMap<std::unordered_map<EndpointId, std::unique_ptr<T>>, U> : std::is_base_of<T, U> {};

} // namespace anonymous

FastRtpsComAdapter::FastRtpsComAdapter(cfg::Config config, const std::string& participantName)
    : _config{std::move(config)}
    , _participantName(participantName)
{
    _participant = &get_by_name(_config.simulationSetup.participants, participantName);
    _participantId = _participant->id;
}

void FastRtpsComAdapter::joinIbDomain(uint32_t domainId)
{
    joinDomain(domainId);
}

void FastRtpsComAdapter::joinDomain(uint32_t domainId)
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

        std::cout << "FastRtpsComAdapter is using DiscoverType: " << fastRtpsCfg.discoveryType << "\n";
        switch (fastRtpsCfg.discoveryType)
        {
        case cfg::FastRtps::DiscoveryType::Local:
        {
            Locator_t unicastLocator;
            pParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(unicastLocator);

            for (auto&& participant : _config.simulationSetup.participants)
            {
                if (participant.id == _participantId)
                    continue;

                Locator_t participantLocator;
                auto port = CalculateMetaTrafficPort(participant.id);
                IPLocator::createLocator(LOCATOR_KIND_UDPv4, "127.0.0.1", port, participantLocator);
                pParam.rtps.builtin.initialPeersList.push_back(participantLocator);
            }
            break;
        }

        case cfg::FastRtps::DiscoveryType::Unicast:
        {
            Locator_t unicastLocator;
            pParam.rtps.builtin.metatrafficUnicastLocatorList.push_back(unicastLocator);

            for (auto&& participant : _config.simulationSetup.participants)
            {
                if (participant.id == _participantId)
                    continue;

                Locator_t participantLocator;
                auto port = CalculateMetaTrafficPort(participant.id);
                try
                {
                    auto&& participantIp = fastRtpsCfg.unicastLocators.at(participant.name);
                    IPLocator::createLocator(LOCATOR_KIND_UDPv4, participantIp, port, participantLocator);
                }
                catch (const std::out_of_range&)
                {
                    throw cfg::Misconfiguration{"No UnicastLocator configured for participant \"" + participant.name + "\""};
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

void FastRtpsComAdapter::registerTopicTypeIfNecessary(TopicDataType* topicType)
{
    auto* participant = _fastRtpsParticipant.get();
    TopicDataType* registeredType = nullptr;
    if (!Domain::getRegisteredType(participant, topicType->getName(), &registeredType))
        Domain::registerType(participant, topicType);

    assert(Domain::getRegisteredType(participant, topicType->getName(), &registeredType));
    assert(registeredType);
}

template <class AttrT>
void FastRtpsComAdapter::SetupPubSubAttributes(AttrT& attributes, const std::string& topicName, TopicDataType* topicType)
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

auto FastRtpsComAdapter::MakeFastrtpsProfileName(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType) -> std::string
{
    std::string shortTypeName{topicType->getName()};
    auto colonPos = shortTypeName.rfind(':');
    if (colonPos != std::string::npos)
    {
        shortTypeName = shortTypeName.substr(colonPos + 1);
    }
    return _participantName + '-' + shortTypeName + '-' + topicName;
}

auto FastRtpsComAdapter::createPublisher(const std::string& topicName, TopicDataType* topicType, PublisherListener* listener) -> Publisher*
{
    assert(_fastRtpsParticipant);

    registerTopicTypeIfNecessary(topicType);

    bool usingStrictSyncPolicy = (_config.simulationSetup.timeSync.syncPolicy == cfg::TimeSync::SyncPolicy::Strict);

    Publisher* publisher;
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
        if (usingStrictSyncPolicy)
        {
            auto tickPeriod = std::chrono::duration_cast<std::chrono::duration<long double, std::ratio<1>>>(_config.simulationSetup.timeSync.tickPeriod);
            auto heartBeatPeriod = tickPeriod / 10.0;

            pubAttributes.times.heartbeatPeriod = Time_t{heartBeatPeriod.count()};
        }
        publisher = Domain::createPublisher(_fastRtpsParticipant.get(), pubAttributes, listener);
    }

    if (publisher == nullptr)
        throw std::exception();

    if (usingStrictSyncPolicy)
        _publishersToWaitFor.push_back(publisher);

    return publisher;
}

auto FastRtpsComAdapter::createSubscriber(const std::string& topicName, TopicDataType* topicType, SubscriberListener* listener) -> Subscriber*
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
void FastRtpsComAdapter::PublishRtpsTopic(const std::string& topicName, EndpointId endpointId)
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
void FastRtpsComAdapter::SubscribeRtpsTopic(const std::string& topicName, IIbMessageReceiver<to_ib_message_t<IdlMessageT>>* receiver)
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
void FastRtpsComAdapter::PublishRtpsTopics(const std::string& topicName, EndpointId endpointId)
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
void FastRtpsComAdapter::SubscribeRtpsTopics(const std::string& topicName, EndpointT* receiver)
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

auto FastRtpsComAdapter::CreateCanController(const std::string& canonicalName) -> can::ICanController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->canControllers, canonicalName);

    if (useNetworkSimulator())
    {
        return CreateControllerForLink<can::CanControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<can::CanController>(config);
    }
}

auto FastRtpsComAdapter::CreateEthController(const std::string& canonicalName) -> eth::IEthController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->ethernetControllers, canonicalName);
    if (useNetworkSimulator())
    {
        return CreateControllerForLink<eth::EthControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<eth::EthController>(config);
    }
}

auto FastRtpsComAdapter::CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->flexrayControllers, canonicalName);
    if (useNetworkSimulator())
    {
        return CreateControllerForLink<fr::FrControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<fr::FrController>(config);
    }
}

auto FastRtpsComAdapter::CreateLinController(const std::string& canonicalName) -> lin::ILinController*
{
    assert(_participant);

    auto&& config = get_by_name(_participant->linControllers, canonicalName);
    if (useNetworkSimulator())
    {
        return CreateControllerForLink<lin::LinControllerProxy>(config);
    }
    else
    {
        return CreateControllerForLink<lin::LinController>(config);
    }
}

auto FastRtpsComAdapter::CreateAnalogIn(const std::string& canonicalName) -> sim::io::IAnalogInPort*
{
    auto&& config = get_by_name(_participant->analogIoPorts, canonicalName);
    return CreateInPort<io::AnalogIoMessage>(config);
}

auto FastRtpsComAdapter::CreateDigitalIn(const std::string& canonicalName) -> sim::io::IDigitalInPort*
{
    auto&& config = get_by_name(_participant->digitalIoPorts, canonicalName);
    return CreateInPort<io::DigitalIoMessage>(config);
}

auto FastRtpsComAdapter::CreatePwmIn(const std::string& canonicalName) -> sim::io::IPwmInPort*
{
    auto&& config = get_by_name(_participant->pwmPorts, canonicalName);
    return CreateInPort<io::PwmIoMessage>(config);
}

auto FastRtpsComAdapter::CreatePatternIn(const std::string& canonicalName) -> sim::io::IPatternInPort*
{
    auto&& config = get_by_name(_participant->patternPorts, canonicalName);
    return CreateInPort<io::PatternIoMessage>(config);
}

auto FastRtpsComAdapter::CreateAnalogOut(const std::string& canonicalName) -> sim::io::IAnalogOutPort*
{
    auto&& config = get_by_name(_participant->analogIoPorts, canonicalName);
    return CreateOutPort<io::AnalogIoMessage>(config);
}

auto FastRtpsComAdapter::CreateDigitalOut(const std::string& canonicalName) -> sim::io::IDigitalOutPort*
{
    auto&& config = get_by_name(_participant->digitalIoPorts, canonicalName);
    return CreateOutPort<io::DigitalIoMessage>(config);
}

auto FastRtpsComAdapter::CreatePwmOut(const std::string& canonicalName) -> sim::io::IPwmOutPort*
{
    auto&& config = get_by_name(_participant->pwmPorts, canonicalName);
    return CreateOutPort<io::PwmIoMessage>(config);
}

auto FastRtpsComAdapter::CreatePatternOut(const std::string& canonicalName) -> sim::io::IPatternOutPort*
{
    auto&& config = get_by_name(_participant->patternPorts, canonicalName);
    return CreateOutPort<io::PatternIoMessage>(config);
}

template<class MsgT, class ConfigT>
auto FastRtpsComAdapter::CreateInPort(const ConfigT& config) -> io::IInPort<MsgT>*
{
    if (config.direction != cfg::PortDirection::In)
        throw std::runtime_error("Invalid port direction!");

    return CreateControllerForLink<io::InPort<MsgT>>(config, config);
}

template<class MsgT, class ConfigT>
auto FastRtpsComAdapter::CreateOutPort(const ConfigT& config) -> io::IOutPort<MsgT>*
{
    if (config.direction != cfg::PortDirection::Out)
        throw std::runtime_error("Invalid port direction!");

    auto port = CreateControllerForLink<io::OutPort<MsgT>>(config, config);
    port->Write(config.initvalue, std::chrono::nanoseconds{0});

    return port;
}

auto FastRtpsComAdapter::CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher*
{
    auto&& config = get_by_name(_participant->genericPublishers, canonicalName);
    return CreateControllerForLink<sim::generic::GenericPublisher>(config, config);
}

auto FastRtpsComAdapter::CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber*
{
    auto&& config = get_by_name(_participant->genericSubscribers, canonicalName);
    return CreateControllerForLink<sim::generic::GenericSubscriber>(config, config);
}

auto FastRtpsComAdapter::CreateSyncMaster() -> sync::ISyncMaster*
{
    assert(_participant);

    if (!isSyncMaster())
    {
        std::cerr << "Error: Participant " << _participant->name << " is not configured as SyncMaster!" << std::endl;
        throw std::runtime_error("Participant not configured as SyncMaster");
    }

    auto* systemMonitor = GetSystemMonitor();
    return CreateController<sync::SyncMaster>(1027, "default", _config, systemMonitor);
}

auto FastRtpsComAdapter::GetParticipantController() -> sync::IParticipantController*
{
    auto&& controllers = std::get<ControllerMap<sync::IParticipantController>>(_controllers);
    if (controllers.begin() != controllers.end())
    {
        return controllers.begin()->second.get();
    }
    else
    {
        return CreateController<sync::ParticipantController>(1024, "default", *_participant);
    }

}

auto FastRtpsComAdapter::GetSystemMonitor() -> sync::ISystemMonitor*
{
    auto&& monitors = std::get<ControllerMap<sync::ISystemMonitor>>(_controllers);
    if (monitors.begin() != monitors.end())
    {
        return monitors.begin()->second.get();
    }
    else
    {
        return CreateController<sync::SystemMonitor>(1025, "default", _config.simulationSetup);
    }
}

auto FastRtpsComAdapter::GetSystemController() -> sync::ISystemController*
{
    auto&& controllers = std::get<ControllerMap<sync::ISystemController>>(_controllers);
    if (controllers.begin() != controllers.end())
    {
        return controllers.begin()->second.get();
    }
    else
    {
        return CreateController<sync::SystemController>(1026, "default");
    }
}

void FastRtpsComAdapter::RegisterCanSimulator(can::IIbToCanSimulator* busSim)
{
    RegisterSimulator(busSim);
}

void FastRtpsComAdapter::RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim)
{
    RegisterSimulator(busSim);
}

void FastRtpsComAdapter::RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim)
{
    RegisterSimulator(busSim);
}

void FastRtpsComAdapter::RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim)
{
    RegisterSimulator(busSim);
}


void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const can::CanMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, can::CanMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const can::CanTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const can::CanControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const can::CanConfigureBaudrate& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const can::CanSetControllerMode& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const eth::EthMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, eth::EthMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const eth::EthTransmitAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const eth::EthStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const eth::EthSetMode& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::FrMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, sim::fr::FrMessage&& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::FrMessageAck& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, sim::fr::FrMessageAck&& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::FrSymbol& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::FrSymbolAck& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::HostCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::TxBufferUpdate& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::fr::ControllerStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::LinMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::RxRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::TxAcknowledge& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::WakeupRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::ControllerConfig& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::SlaveConfiguration& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::lin::SlaveResponse& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::io::AnalogIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::io::DigitalIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::io::PatternIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, sim::io::PatternIoMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::io::PwmIoMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sim::generic::GenericMessage& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, sim::generic::GenericMessage&& msg)
{
    SendIbMessageImpl(from, std::move(msg));
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::Tick& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::TickDone& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::QuantumRequest& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::QuantumGrant& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::ParticipantStatus& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::ParticipantCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

void FastRtpsComAdapter::SendIbMessage(EndpointAddress from, const sync::SystemCommand& msg)
{
    SendIbMessageImpl(from, msg);
}

template<typename IbMessageT>
void FastRtpsComAdapter::SendIbMessageImpl(EndpointAddress from, IbMessageT&& msg)
{
    auto idlMsg = to_idl(std::forward<IbMessageT>(msg));
    idlMsg.senderAddr(to_idl(from));

    auto& rtpsTopics = std::get<RtpsTopics<decltype(idlMsg)>>(_rtpsTopics);
    assert(rtpsTopics.endpointToPublisherMap.find(from.endpoint) != rtpsTopics.endpointToPublisherMap.end());

    auto* publisher = rtpsTopics.endpointToPublisherMap[from.endpoint];
    publisher->write(&idlMsg);
}

template<class ControllerT, typename... Arg>
auto FastRtpsComAdapter::CreateController(EndpointId endpointId, const std::string& topicname, Arg&&... arg) -> ControllerT*
{
    auto&& controllerMap = tt::predicative_get<tt::rbind<IsControllerMap, ControllerT>::template type>(_controllers);
    if (controllerMap.count(endpointId))
    {
        std::cerr << "ERROR: FastRtpsComAdapter already has a controller with endpointid=" << endpointId << std::endl;
        throw std::runtime_error("Duplicate EndpointId");
    }

    auto controller = std::make_unique<ControllerT>(this, std::forward<Arg>(arg)...);
    auto controllerPtr = controller.get();
    controller->SetEndpointAddress(EndpointAddress{_participantId, endpointId});

    PublishRtpsTopics<ControllerT>(topicname, endpointId);
    SubscribeRtpsTopics(topicname, controllerPtr);

    controllerMap[endpointId] = std::move(controller);
    return controllerPtr;
}

auto FastRtpsComAdapter::GetLinkById(int16_t linkId) -> cfg::Link&
{
    for (auto&& link : _config.simulationSetup.links)
    {
        if (link.id == linkId)
            return link;
    }

    throw cfg::Misconfiguration("Invalid linkId " + std::to_string(linkId));
}

template<class ControllerT, class ConfigT, typename... Arg>
auto FastRtpsComAdapter::CreateControllerForLink(const ConfigT& config, Arg&&... arg) -> ControllerT*
{
    auto&& linkCfg = GetLinkById(config.linkId);
    return CreateController<ControllerT>(config.endpointId, linkCfg.name, std::forward<Arg>(arg)...);
}


template<class IIbToSimulatorT>
void FastRtpsComAdapter::RegisterSimulator(IIbToSimulatorT* busSim)
{
    auto&& simulator = std::get<IIbToSimulatorT*>(_simulators);
    if (simulator)
    {
        std::cerr << "ERROR: A " << typeid(IIbToSimulatorT).name() << " is already registered" << std::endl;
        return;
    }

    std::unordered_map<std::string, mw::EndpointId> endpointMap;
    auto addToEndpointMap = [&endpointMap](auto&& participantName, auto&& controllerConfigs)
    {
        for (auto&& cfg : controllerConfigs)
        {
            std::string qualifiedName = participantName + "/" + cfg.name;
            endpointMap[qualifiedName] = cfg.endpointId;
        }
    };

    for (auto&& participant : _config.simulationSetup.participants)
    {
        addToEndpointMap(participant.name, participant.canControllers);
        addToEndpointMap(participant.name, participant.linControllers);
        addToEndpointMap(participant.name, participant.ethernetControllers);
        addToEndpointMap(participant.name, participant.flexrayControllers);
    }
    for (auto&& ethSwitch : _config.simulationSetup.switches)
    {
        addToEndpointMap(ethSwitch.name, ethSwitch.ports);
    }


    // get_by_name throws if the current node is not configured as a network simulator.
    for (auto&& simulatorName : _participant->networkSimulators)
    {
        auto&& simulatorConfig = get_by_name(_config.simulationSetup.networkSimulators, simulatorName);

        for (auto&& linkName : simulatorConfig.simulatedLinks)
        {
            for (auto&& endpointName : get_by_name(_config.simulationSetup.links, linkName).endpoints)
            {
                try
                {
                    auto proxyEndpoint = endpointMap.at(endpointName);
                    PublishRtpsTopics<IIbToSimulatorT>(linkName, proxyEndpoint);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "ERROR: Cannot register simulator topics for link \"" << linkName << "\": " << e.what() << std::endl;
                    continue;
                }
            }
            SubscribeRtpsTopics(linkName, busSim);
        }
    }


    simulator = busSim;
}

bool FastRtpsComAdapter::useNetworkSimulator() const
{
    return !_config.simulationSetup.networkSimulators.empty();
}

bool FastRtpsComAdapter::isSyncMaster() const
{
    assert(_participant);

    return _participant->isSyncMaster;
}

void FastRtpsComAdapter::WaitForMessageDelivery()
{
    for (auto publisher : _publishersToWaitFor)
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

} // namespace mw
} // namespace ib
