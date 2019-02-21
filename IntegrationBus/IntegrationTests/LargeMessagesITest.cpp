// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class LargeMessagesITest : public testing::Test
{
protected:
    LargeMessagesITest()
        : domainId{static_cast<uint32_t>(GetTestPid())}
    {
        topic.name = "LargeDataBlobTopic";

        ibConfig = ib::cfg::Config::FromJsonFile("LargeMessagesITest_IbConfig.json");
        
        pubComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Publisher", domainId);
        subComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Subscriber", domainId);
    }

    void Subscribe()
    {
        topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
        topic.subscriber->SetReceiveMessageHandler(
            [&topic = topic](auto* /*subscriber*/, auto&& data)
            {
                topic.reply.set_value(data);
            }
        );
    }

    void Publish(std::vector<uint8_t>& data)
    {
        topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
        topic.publisher->Publish(data);
    }

    struct Topic
    {
        std::string name;
        std::promise<std::vector<uint8_t>> reply;
        ib::sim::generic::IGenericPublisher* publisher;
        ib::sim::generic::IGenericSubscriber* subscriber;
    };

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    Topic topic;

    std::unique_ptr<ib::mw::IComAdapter> pubComAdapter;
    std::unique_ptr<ib::mw::IComAdapter> subComAdapter;
};
    
TEST_F(LargeMessagesITest, publish_and_subscribe_large_messages)
{
    Subscribe();

    // https://asc..com/jira/browse/AFTMAGT-121
    // Maximum payload size is 65416, beyond that we are testing the ASYNCHRONOUS_PUBLISH_MODE of FastRTPS.
    size_t sizeInBytes = 114793;
    std::vector<uint8_t> data(sizeInBytes, 'D');

    std::thread publishThread{[this, &data]() { this->Publish(data); }};

    auto&& reply = topic.reply.get_future();
    auto ready = reply.wait_for(5s);
    ASSERT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(reply.get(), data);
    
    publishThread.join();
}

} // anonymous namespace
