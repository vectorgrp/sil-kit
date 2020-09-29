// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "VAsioRegistry.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class GenericMessageITest : public testing::Test
{
protected:
    GenericMessageITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());
    }


    void Subscribe()
    {
        auto subComAdapter = CreateComAdapterImpl(ibConfig, "Subscriber");
        subComAdapter->joinIbDomain(domainId);

        std::promise<void> threadFinishedPromise{};

        unsigned int receiveCount{0};

        for (auto&& topic: topics)
        {
            auto subscriber = subComAdapter->CreateGenericSubscriber(topic.name);

            subscriber->SetReceiveMessageHandler(
                [&topic = topic, &threadFinishedPromise, &receiveCount, this](auto* /*subscriber*/, const auto& data)
                {
                    topic.receivedData = data;

                    if (topics.size() == ++receiveCount)
                    {
                        allReceivedPromise.set_value();
                        threadFinishedPromise.set_value();
                    }
                }
            );
        }

        auto threadFinished = threadFinishedPromise.get_future();
        threadFinished.wait_for(10min);
    }

    void Publish()
    {
        auto pubComAdapter = CreateComAdapterImpl(ibConfig, "Publisher");
        pubComAdapter->joinIbDomain(domainId);

        for (auto&& topic: topics)
        {
            auto publisher = pubComAdapter->CreateGenericPublisher(topic.name);
            std::this_thread::sleep_for(200ms);
            publisher->Publish(topic.expectedData);
        }

        auto allReceived = allReceivedPromise.get_future();
        allReceived.wait_for(10min);
    }

    struct Topic
    {
        std::string name;
        std::vector<uint8_t> expectedData;
        std::vector<uint8_t> receivedData;
    };

protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<Topic> topics;

    std::promise<void> allReceivedPromise;
};

TEST_F(GenericMessageITest, DISABLED_publish_and_subscribe_generic_messages_fastrtps)
{
    topics.resize(2);
    topics[0].name = "GroundTruth";
    topics[0].expectedData = std::vector<uint8_t>{topics[0].name.begin(), topics[0].name.end()};
    topics[1].name = "VehicleModelOut";
    topics[1].expectedData = std::vector<uint8_t>{topics[1].name.begin(), topics[1].name.end()};

    ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    std::thread subscribeThread{[this] { Subscribe(); }};
    std::thread publishThread{[this]{ Publish(); }};

    publishThread.join();
    subscribeThread.join();

    // Test expectations
    for (auto&& topic : topics)
    {
        EXPECT_EQ(topic.expectedData, topic.receivedData);
    }
}

TEST_F(GenericMessageITest, DISABLED_publish_and_subscribe_large_messages_fastrtps)
{
    topics.resize(1);
    topics[0].name = "LargeDataBlobTopic";
    // Maximum payload size is 65416, beyond that we are testing the ASYNCHRONOUS_PUBLISH_MODE of FastRTPS.
    size_t sizeInBytes = 114793;
    topics[0].expectedData = std::vector<uint8_t>(sizeInBytes, 'D');

    ibConfig = ib::cfg::Config::FromJsonFile("LargeMessagesITest_IbConfig.json");
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::FastRTPS;

    std::thread subscribeThread{[this] { Subscribe(); }};
    std::thread publishThread{[this] { Publish(); }};

    publishThread.join();
    subscribeThread.join();

    // Test expectations
    for (auto&& topic : topics)
    {
        EXPECT_EQ(topic.expectedData, topic.receivedData);
    }
}

TEST_F(GenericMessageITest, publish_and_subscribe_generic_messages_vasio)
{
    topics.resize(2);
    topics[0].name = "GroundTruth";
    topics[0].expectedData = std::vector<uint8_t>{topics[0].name.begin(), topics[0].name.end()};
    topics[1].name = "VehicleModelOut";
    topics[1].expectedData = std::vector<uint8_t>{topics[1].name.begin(), topics[1].name.end()};

    ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    std::thread subscribeThread{[this] { Subscribe(); }};
    std::thread publishThread{[this] { Publish(); }};

    publishThread.join();
    subscribeThread.join();

    // Test expectations
    for (auto&& topic : topics)
    {
        EXPECT_EQ(topic.expectedData, topic.receivedData);
    }
}

TEST_F(GenericMessageITest, publish_and_subscribe_large_messages_vasio)
{
    topics.resize(1);
    topics[0].name = "LargeDataBlobTopic";
    // Maximum payload size is 65416, beyond that we are testing the ASYNCHRONOUS_PUBLISH_MODE of FastRTPS.
    size_t sizeInBytes = 114793;
    topics[0].expectedData = std::vector<uint8_t>(sizeInBytes, 'D');

    ibConfig = ib::cfg::Config::FromJsonFile("LargeMessagesITest_IbConfig.json");
    ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;

    auto registry = std::make_unique<VAsioRegistry>(ibConfig);
    registry->ProvideDomain(domainId);

    std::thread subscribeThread{[this] { Subscribe(); }};
    std::thread publishThread{[this] { Publish(); }};

    publishThread.join();
    subscribeThread.join();

    // Test expectations
    for (auto&& topic : topics)
    {
        EXPECT_EQ(topic.expectedData, topic.receivedData);
    }
}

} // anonymous namespace
