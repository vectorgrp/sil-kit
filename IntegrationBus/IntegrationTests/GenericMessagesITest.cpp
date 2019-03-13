// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

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

class GenericMessageITest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveData, void(ib::sim::generic::IGenericSubscriber*, const std::vector<uint8_t>&));
    };

protected:
    GenericMessageITest()
        : domainId{static_cast<uint32_t>(GetTestPid())}
        , topics(2)
    {
        topics[0].name = "GroundTruth";
        topics[1].name = "VehicleModelOut";

        ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");
        
        pubComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Publisher", domainId);
        subComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Subscriber", domainId);
    }

    void Subscribe()
    {
        for (auto&& topic: topics)
        {
            topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
            topic.subscriber->SetReceiveMessageHandler(
                [&topic = topic, &callbacks = callbacks](auto* subscriber, auto&& data)
                {
                    callbacks.ReceiveData(subscriber, data);
                    topic.reply.set_value(std::string{data.begin(), data.end()});
                }
            );

            std::vector<uint8_t> expectedPayload{topic.name.begin(), topic.name.end()};
            EXPECT_CALL(callbacks, ReceiveData(topic.subscriber, expectedPayload));
        }
    }

    void Publish()
    {
        for (auto&& topic: topics)
        {
            topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
            topic.publisher->Publish({topic.name.begin(), topic.name.end()});
        }
    }

    struct Topic
    {
        std::string name;
        std::promise<std::string> reply;
        ib::sim::generic::IGenericPublisher* publisher;
        ib::sim::generic::IGenericSubscriber* subscriber;
    };

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    Callbacks callbacks;
    std::vector<Topic> topics;

    std::unique_ptr<ib::mw::IComAdapter> pubComAdapter;
    std::unique_ptr<ib::mw::IComAdapter> subComAdapter;
};
    
TEST_F(GenericMessageITest, publish_and_subscribe_generic_messages)
{
    Subscribe();

    std::thread publishThread{[this]() { this->Publish(); }};

    for (auto&& topic : topics)
    {
        auto&& reply = topic.reply.get_future();
        auto ready = reply.wait_for(10min);
        ASSERT_EQ(ready, std::future_status::ready);
        EXPECT_EQ(reply.get(), topic.name);
    }
    
    publishThread.join();
}

} // anonymous namespace
