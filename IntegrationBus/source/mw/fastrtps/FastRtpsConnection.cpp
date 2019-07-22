// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FastRtpsConnection.hpp"

#include <cassert>
#include <sstream>
#include <future>

#include "fastrtps/Domain.h"
#include "fastrtps/utils/IPLocator.h"

#include "ib/cfg/string_utils.hpp"


namespace ib {
namespace mw {

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

//using namespace ib::sim;

namespace tt = util::tuple_tools;

FastRtpsConnection::FastRtpsConnection(cfg::Config config, std::string participantName, ParticipantId participantId)
    : _config{std::move(config)}
    , _participantName{std::move(participantName)}
    , _participantId{participantId}
{
}

void FastRtpsConnection::JoinDomain(uint32_t domainId)
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
        auto domainLock{FastRtps::GetFastRtpsDomainLock()};
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

        std::cout << "FastRtpsConnection is using DiscoverType: " << fastRtpsCfg.discoveryType << "\n";
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

        auto domainLock{FastRtps::GetFastRtpsDomainLock()};
        participant = Domain::createParticipant(pParam);
    }

    if (participant == nullptr)
        throw std::exception();

    _fastRtpsParticipant = std::unique_ptr<eprosima::fastrtps::Participant, FastRtps::RemoveParticipant>(participant);
}

void FastRtpsConnection::registerTopicTypeIfNecessary(TopicDataType* topicType)
{
    auto* participant = _fastRtpsParticipant.get();
    TopicDataType* registeredType = nullptr;
    if (!Domain::getRegisteredType(participant, topicType->getName(), &registeredType))
        Domain::registerType(participant, topicType);

    assert(Domain::getRegisteredType(participant, topicType->getName(), &registeredType));
    assert(registeredType);
}

auto FastRtpsConnection::MakeFastrtpsProfileName(const std::string& topicName, eprosima::fastrtps::TopicDataType* topicType) -> std::string
{
    std::string shortTypeName{topicType->getName()};
    auto colonPos = shortTypeName.rfind(':');
    if (colonPos != std::string::npos)
    {
        shortTypeName = shortTypeName.substr(colonPos + 1);
    }
    return _participantName + '-' + shortTypeName + '-' + topicName;
}

auto FastRtpsConnection::createPublisher(const std::string& topicName, TopicDataType* topicType, PublisherListener* listener) -> Publisher*
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
        // only send an acknowledgment in reply to a heartbeat. Thus, the heartbeat
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

auto FastRtpsConnection::createSubscriber(const std::string& topicName, TopicDataType* topicType, SubscriberListener* listener) -> Subscriber*
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

void FastRtpsConnection::OnAllMessagesDelivered(std::function<void(void)> callback)
{
    _messagesDelivered = std::async(std::launch::async,
        [this, callback]() {
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
                auto allAcked = publisher->wait_for_all_acked(Time_t{ 1200, 0 });
                assert(allAcked);
            }

            callback();
        }
    );
}

void FastRtpsConnection::FlushSendBuffers()
{
    for (auto publisher : _allPublishers)
    {
        publisher->removeAllChange(nullptr);
    }
}

void FastRtpsConnection::RegisterNewPeerCallback(std::function<void()> /*callback*/)
{
}

} // namespace mw
} // namespace ib
