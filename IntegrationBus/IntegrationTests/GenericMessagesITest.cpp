// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"
#include "Registry.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::registry;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class GenericMessageITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveData, void(ib::sim::generic::IGenericSubscriber*, const std::vector<uint8_t>&));
    };

protected:
    GenericMessageITest()
        : topics(2)
    {
    }

    struct Topic
    {
        std::string name;
        std::promise<std::vector<uint8_t>> reply;
        ib::sim::generic::IGenericPublisher* publisher;
        ib::sim::generic::IGenericSubscriber* subscriber;
    };

protected:
    Callbacks callbacks;
    std::vector<Topic> topics;
};
    
TEST_F(GenericMessageITest, publish_and_subscribe_generic_messages)
{
    // Test setup
    topics[0].name = "GroundTruth";
    topics[1].name = "VehicleModelOut";
    auto ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");

    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    auto pubComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Publisher");
    pubComAdapter->joinIbDomain(domainId);

    auto subComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Subscriber");
    subComAdapter->joinIbDomain(domainId);

    // Subscribe Generic Messages
    for (auto&& topic : topics)
    {
        topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
        topic.subscriber->SetReceiveMessageHandler(
            [&topic = topic, &callbacks = callbacks](auto* subscriber, auto&& data) {
            callbacks.ReceiveData(subscriber, data);
            topic.reply.set_value(data);
        });

        std::vector<uint8_t> expectedPayload{topic.name.begin(), topic.name.end()};
        EXPECT_CALL(callbacks, ReceiveData(topic.subscriber, expectedPayload));
    }

    // Publish Generic Messages
    std::thread publishThread{[&topics = topics, &pubComAdapter = pubComAdapter]() {
        for (auto&& topic : topics)
        {
            topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
            topic.publisher->Publish({topic.name.begin(), topic.name.end()});
        }
    }};

    // Test expectations
    for (auto&& topic : topics)
    {
        auto&& reply = topic.reply.get_future();
        auto ready = reply.wait_for(10min);
        ASSERT_EQ(ready, std::future_status::ready);

        auto&& replyData = reply.get();
        EXPECT_EQ(std::string(replyData.begin(), replyData.end()), topic.name);
    }
    
    publishThread.join();
}

TEST_F(GenericMessageITest, publish_and_subscribe_large_messages)
{
    // Test setup
    topics[0].name = "LargeDataBlobTopic";
    auto ibConfig = ib::cfg::Config::FromJsonFile("LargeMessagesITest_IbConfig.json");

    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    auto pubComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Publisher");
    pubComAdapter->joinIbDomain(domainId);

    auto subComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Subscriber");
    subComAdapter->joinIbDomain(domainId);

    // Maximum payload size is 65416, beyond that we are testing the ASYNCHRONOUS_PUBLISH_MODE of FastRTPS.
    size_t sizeInBytes = 114793;
    std::vector<uint8_t> data(sizeInBytes, 'D');

    // Subscribe Generic Message
    auto&& topic = topics[0];

    topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
    topic.subscriber->SetReceiveMessageHandler(
        [&topic = topic, &callbacks = callbacks](auto* subscriber, auto&& data) {
        callbacks.ReceiveData(subscriber, data);
        topic.reply.set_value(data);
    });

    EXPECT_CALL(callbacks, ReceiveData(topic.subscriber, data));

    // Publish large Generic Message
    topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
    std::thread publishThread{[&topic = topic, &data = data]() {
        topic.publisher->Publish(data);
    }};

    // Test expectations
    auto&& reply = topic.reply.get_future();
    auto ready = reply.wait_for(10min);
    ASSERT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(reply.get(), data);

    publishThread.join();
}

TEST_F(GenericMessageITest, publish_and_subscribe_generic_messages_vasio)
{
    // Test setup
    topics[0].name = "GroundTruth";
    topics[1].name = "VehicleModelOut";
    auto ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");

    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    auto registry = std::make_unique<Registry>(ibConfig);
    registry->ProvideDomain(domainId);

    auto pubComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Publisher");
    pubComAdapter->joinIbDomain(domainId);

    auto subComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Subscriber");
    subComAdapter->joinIbDomain(domainId);

    // Subscribe Generic Messages
    for (auto&& topic : topics)
    {
        topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
        topic.subscriber->SetReceiveMessageHandler(
            [&topic = topic, &callbacks = callbacks](auto* subscriber, auto&& data) {
            callbacks.ReceiveData(subscriber, data);
            topic.reply.set_value(data);
        });

        std::vector<uint8_t> expectedPayload{topic.name.begin(), topic.name.end()};
        EXPECT_CALL(callbacks, ReceiveData(topic.subscriber, expectedPayload));
    }

    std::this_thread::sleep_for(200ms);

    // Publish Generic Messages
    std::thread publishThread{[&topics = topics, &pubComAdapter = pubComAdapter]() {
        for (auto&& topic : topics)
        {
            topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
            topic.publisher->Publish({topic.name.begin(), topic.name.end()});
        }
    }};

    // Test expectations
    for (auto&& topic : topics)
    {
        auto&& reply = topic.reply.get_future();
        auto ready = reply.wait_for(10s);
        ASSERT_EQ(ready, std::future_status::ready);

        auto&& replyData = reply.get();
        EXPECT_EQ(std::string(replyData.begin(), replyData.end()), topic.name);
    }

    publishThread.join();
}

TEST_F(GenericMessageITest, publish_and_subscribe_large_messages_vasio)
{
    // Test setup
    topics[0].name = "GroundTruth";
    topics[1].name = "VehicleModelOut";
    auto ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");

    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    auto registry = std::make_unique<Registry>(ibConfig);
    registry->ProvideDomain(domainId);

    auto pubComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Publisher");
    pubComAdapter->joinIbDomain(domainId);

    auto subComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Subscriber");
    subComAdapter->joinIbDomain(domainId);

    // Large Message
    size_t sizeInBytes = 114793;
    std::vector<uint8_t> data(sizeInBytes, 'D');

    // Subscribe Generic Message
    auto&& topic = topics[0];

    topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
    topic.subscriber->SetReceiveMessageHandler(
        [&topic = topic, &callbacks = callbacks](auto* subscriber, auto&& data) {
        callbacks.ReceiveData(subscriber, data);
        topic.reply.set_value(data);
    });

    EXPECT_CALL(callbacks, ReceiveData(topic.subscriber, data));

    std::this_thread::sleep_for(200ms);

    // Publish large Generic Message
    topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
    std::thread publishThread{[&topic = topic, &data = data]() {
        topic.publisher->Publish(data);
    }};

    // Test expectations
    auto&& reply = topic.reply.get_future();
    auto ready = reply.wait_for(10s);
    ASSERT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(reply.get(), data);

    publishThread.join();
}

} // anonymous namespace
