// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriber.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/util/functional.hpp"

#include "MockParticipant.hpp"

#include "DataPublisher.hpp"
#include "DataMessageDatatypeUtils.hpp"

#include "YamlParser.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::PubSub;

using ::SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(Services::PubSub::DataSubscriberInternal*, CreateDataSubscriberInternal,
                (const std::string& /*topic*/, const std::string& /*linkName*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>&)/*publisherLabels*/,
                 Services::PubSub::DataMessageHandlerT /*callback*/, Services::PubSub::IDataSubscriber* /*parent*/),
                (override));
};

class DataSubscriberTest : public ::testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, ReceiveDataDefault, (IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
        MOCK_METHOD(void, ReceiveDataExplicitA, (IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
        MOCK_METHOD(void, ReceiveDataExplicitB, (IDataSubscriber*, const DataMessageEvent& dataMessageEvent));

        MOCK_METHOD(void, NewDataPublisher, (IDataSubscriber*, const NewDataPublisherEvent& newDataPublisherEvent));
    };

protected:
    DataSubscriberTest()
        : subscriber{&participant,
                     participant.GetTimeProvider(),
                     topic,
                     mediaType,
                     labels,
                     SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault),
                     SilKit::Util::bind_method(&callbacks, &Callbacks::NewDataPublisher)}
        , publisher{&participant, participant.GetTimeProvider(), topic, mediaType, labels, publisherUuid}
    {
        subscriber.SetServiceDescriptor(from_endpointAddress(subscriberEndpointAddress));
        SetupPublisherServiceDescriptor(publisher, publisherUuid, publisherEndpointAddress);
    }

private:
    void SetupPublisherServiceDescriptor(DataPublisher& dataPublisher, const std::string& uuid,
                                         const EndpointAddress& endpointAddress)
    {
        auto publisherServiceDescriptor = from_endpointAddress(endpointAddress);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherTopic, topic);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherMediaType, mediaType);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubLabels,
                                                           labelsSerialized);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubUUID, uuid);
        dataPublisher.SetServiceDescriptor(publisherServiceDescriptor);
    }

protected:
    ServiceDescriptor MakePublisherServiceDescriptor(const std::string& uuid, const EndpointAddress& endpointAddress)
    {
        auto publisherServiceDescriptor = from_endpointAddress(endpointAddress);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherTopic, topic);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherMediaType, mediaType);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubLabels,
                                                           labelsSerialized);
        publisherServiceDescriptor.SetSupplementalDataItem(Core::Discovery::supplKeyDataPublisherPubUUID, uuid);
        return publisherServiceDescriptor;
    }

protected:
    const std::vector<uint8_t> sampleData{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    const EndpointAddress subscriberEndpointAddress{4, 5};

    const EndpointAddress publisherEndpointAddress{6, 7};
    const std::string publisherUuid{"pubUUID"};

    const EndpointAddress publisher2EndpointAddress{8, 9};
    const std::string publisher2Uuid{"pubUUID-2"};

    const std::string topic{"Topic"};
    const std::string mediaType{};
    const std::map<std::string, std::string> labels;
    const std::string labelsSerialized{SilKit::Config::Serialize<std::map<std::string, std::string>>(labels)};

    Callbacks callbacks;
    MockParticipant participant;
    DataSubscriber subscriber;
    DataPublisher publisher;
};

struct CreateSubscriberInternalMock
{
    MockParticipant* participant;
    std::unique_ptr<DataSubscriberInternal> dataSubscriberInternal;

    auto operator()(const std::string& topic, const std::string& /*linkName*/, const std::string& mediaType,
                    const std::map<std::string, std::string>& labels, Services::PubSub::DataMessageHandlerT defaultHandler,
                    Services::PubSub::IDataSubscriber* parent) -> DataSubscriberInternal*
    {
        dataSubscriberInternal = std::make_unique<DataSubscriberInternal>(
            participant, participant->GetTimeProvider(), topic, mediaType, labels, std::move(defaultHandler), parent);
        return dataSubscriberInternal.get();
    }
};

TEST_F(DataSubscriberTest, add_remove_explicit_message_handler_through_data_subscriber)
{
    // ------------------------------------------------------------------------------------------------------
    // Register the subscriber with the service discovery and extract the discovery handler
    // ------------------------------------------------------------------------------------------------------

    // will contain the handler created by the DataSubscriber upon registering with the service discovery
    SilKit::Core::Discovery::ServiceDiscoveryHandlerT serviceDiscoveryHandler;

    EXPECT_CALL(participant.mockServiceDiscovery,
                RegisterSpecificServiceDiscoveryHandler(testing::_, Core::Discovery::controllerTypeDataPublisher, topic))
        .Times(1)
        .WillOnce(testing::Invoke([&serviceDiscoveryHandler](SilKit::Core::Discovery::ServiceDiscoveryHandlerT handler,
                                                             const std::string& /*controllerTypeName*/,
                                                             const std::string& /*supplDataValue*/) {
            serviceDiscoveryHandler = std::move(handler);
        }));

    subscriber.RegisterServiceDiscovery();
    ASSERT_TRUE(serviceDiscoveryHandler);

    // ------------------------------------------------------------------------------------------------------
    // Fake the call to the extracted service discovery handler for the two publishers
    // ------------------------------------------------------------------------------------------------------

    CreateSubscriberInternalMock createSubscriberInternalMock{&participant, {}};

    // hook up the call to create an internal subscriber
    EXPECT_CALL(participant,
                CreateDataSubscriberInternal(topic, testing::_, mediaType, labels, testing::_, &subscriber))
        .Times(2)
        .WillRepeatedly(std::ref(createSubscriberInternalMock));

    using SilKit::Core::Discovery::ServiceDiscoveryEvent;

    // create the initial internal subscriber for publisher
    serviceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, publisher.GetServiceDescriptor());
    const auto subscriberInternal = std::move(createSubscriberInternalMock.dataSubscriberInternal);
    ASSERT_TRUE(subscriberInternal);

    // ------------------------------------------------------------------------------------------------------
    // Send messages and trigger the default / explicit message handler(s)
    // ------------------------------------------------------------------------------------------------------

    const DataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveDataDefault(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(testing::_, testing::_)).Times(0);
    subscriberInternal->ReceiveMsg(&publisher, msg);

    const auto handlerIdA = subscriber.AddExplicitDataMessageHandler(
        SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataExplicitA), mediaType, labels);

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(testing::_, testing::_)).Times(0);
    subscriberInternal->ReceiveMsg(&publisher, msg);

    const auto handlerIdB = subscriber.AddExplicitDataMessageHandler(
        SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataExplicitB), mediaType, labels);

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(&subscriber, msg)).Times(1);
    subscriberInternal->ReceiveMsg(&publisher, msg);

    // create the second internal subscriber for publisher
    serviceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
                            MakePublisherServiceDescriptor(publisher2Uuid, publisher2EndpointAddress));
    const auto subscriberInternal2 = std::move(createSubscriberInternalMock.dataSubscriberInternal);
    ASSERT_TRUE(subscriberInternal2);

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(&subscriber, msg)).Times(1);
    subscriberInternal2->ReceiveMsg(&publisher, msg);

    subscriber.RemoveExplicitDataMessageHandler(handlerIdA);

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(&subscriber, msg)).Times(1);
    subscriberInternal->ReceiveMsg(&publisher, msg);

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(&subscriber, msg)).Times(1);
    subscriberInternal2->ReceiveMsg(&publisher, msg);

    subscriber.RemoveExplicitDataMessageHandler(handlerIdB);

    EXPECT_CALL(callbacks, ReceiveDataDefault(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(testing::_, testing::_)).Times(0);
    subscriberInternal->ReceiveMsg(&publisher, msg);

    EXPECT_CALL(callbacks, ReceiveDataDefault(&subscriber, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicitA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicitB(testing::_, testing::_)).Times(0);
    subscriberInternal2->ReceiveMsg(&publisher, msg);
}

} // anonymous namespace
