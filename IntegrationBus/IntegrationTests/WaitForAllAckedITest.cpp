// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>
#include <mutex>

#include "CreateComAdapter.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
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

auto to_payload(uint32_t value)
{
    std::vector<uint8_t> payload;
    payload.resize(sizeof(value));
    *reinterpret_cast<uint32_t*>(payload.data()) = value;
    return payload;
}

auto from_payload(const std::vector<uint8_t>& payload) -> uint32_t
{
    assert(payload.size() == sizeof(uint32_t));
    return *reinterpret_cast<const uint32_t*>(payload.data());
}
    
class WaitForAllAckedITest : public testing::Test
{
protected:
    WaitForAllAckedITest()
        : domainId{static_cast<uint32_t>(GetTestPid())}
        , topics{2}
    {
        topics[0].name = "TestData1";
        topics[1].name = "TestData2";
        
        ib::cfg::ConfigBuilder cfgBuilder("WaitForAllAckedConfig");
        auto&& simulationSetup = cfgBuilder.SimulationSetup();
        simulationSetup.AddParticipant("Sender")
            ->AddGenericPublisher(topics[0].name)
            ->AddGenericPublisher(topics[1].name);
        simulationSetup.AddParticipant("Receiver")
            ->AddGenericSubscriber(topics[0].name)
            ->AddGenericSubscriber(topics[1].name);
        simulationSetup.ConfigureTimeSync().WithStrictSyncPolicy().WithTickPeriod(1ms);

        ibConfig = cfgBuilder.Build();

        pubComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Sender");
        pubComAdapter->joinIbDomain(domainId);

        subComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Receiver");
        subComAdapter->joinIbDomain(domainId);
    }

    struct Topic
    {
        std::string name;
        ib::sim::generic::IGenericPublisher* publisher;
        ib::sim::generic::IGenericSubscriber* subscriber;

        uint32_t sendIdx{0};
        uint32_t receiveIdx{0};

        std::promise<bool> testComplete;
    };

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<Topic> topics;

    std::unique_ptr<IComAdapterInternal> pubComAdapter;
    std::unique_ptr<IComAdapterInternal> subComAdapter;
};
    
TEST_F(WaitForAllAckedITest, no_messages_must_be_lost)
{
    const uint32_t msgCount = 100;
    auto&& topic = topics[0];

    // ------------------------------------------------------------
    // SUBSCRIBE
    topic.subscriber = subComAdapter->CreateGenericSubscriber(topic.name);
    topic.subscriber->SetReceiveMessageHandler(
        [&topic, msgCount](auto* /*subscriber*/, auto&& data)
        {
            uint32_t sendIdx = from_payload(data);
            EXPECT_EQ(sendIdx, topic.receiveIdx++);

            if (topic.receiveIdx == msgCount)
                topic.testComplete.set_value(true);
        }
    );

    // ------------------------------------------------------------
    // PUBLISH
    std::thread publishThread{
        [this, msgCount, &topic]()
        {
            topic.publisher = pubComAdapter->CreateGenericPublisher(topic.name);
            // give publishers and subscribers time to match
            std::this_thread::sleep_for(2s);
            for (auto count = 0u; count < msgCount; count++)
            {
                topic.publisher->Publish(to_payload(topic.sendIdx++));
                std::promise<void> messagesDelivered;
                auto future = messagesDelivered.get_future();
                pubComAdapter->OnAllMessagesDelivered([&messagesDelivered]() {

                    messagesDelivered.set_value();

                });
                future.wait();
            }
        }
    };

    // ------------------------------------------------------------
    // Wait until test is finished
    auto testComplete = topic.testComplete.get_future();
    auto futureStatus = testComplete.wait_for(60s);
    
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(testComplete.get());
    
    publishThread.join();
}

TEST_F(WaitForAllAckedITest, messages_must_be_delivered_in_order)
{
    const uint32_t msgCount = 50;
    std::mutex topicMutex;
    
    // ------------------------------------------------------------
    // SUBSCRIBE
    topics[0].subscriber = subComAdapter->CreateGenericSubscriber(topics[0].name);
    topics[0].subscriber->SetReceiveMessageHandler(
        [msgCount, &topicMutex, &topics = topics](auto* /*subscriber*/, auto&& data)
        {
            std::lock_guard<std::mutex> topicGuard{topicMutex};
            
            uint32_t sendIdx = from_payload(data);
            EXPECT_EQ(sendIdx, topics[0].receiveIdx);
            EXPECT_EQ(sendIdx, topics[1].receiveIdx);

            topics[0].receiveIdx++;

            if (topics[0].receiveIdx == msgCount)
                topics[0].testComplete.set_value(true);
        }
    );
    
    topics[1].subscriber = subComAdapter->CreateGenericSubscriber(topics[1].name);
    topics[1].subscriber->SetReceiveMessageHandler(
        [msgCount, &topicMutex, &topics = topics](auto* /*subscriber*/, auto&& data)
        {
            std::lock_guard<std::mutex> topicGuard{topicMutex};
            
            uint32_t sendIdx = from_payload(data);
            EXPECT_EQ(sendIdx + 1, topics[0].receiveIdx);
            EXPECT_EQ(sendIdx, topics[1].receiveIdx);

            topics[1].receiveIdx++;

            if (topics[1].receiveIdx == msgCount)
                topics[1].testComplete.set_value(true);
        }
    );

    // ------------------------------------------------------------
    // PUBLISH
    std::thread publishThread{
        [this, msgCount]()
        {
            topics[0].publisher = pubComAdapter->CreateGenericPublisher(topics[0].name);
            topics[1].publisher = pubComAdapter->CreateGenericPublisher(topics[1].name);
            // give publishers and subscribers time to match
            std::this_thread::sleep_for(2s);
            for (auto count = 0u; count < msgCount; count++)
            {
                for (int i = 0; i < 2; i++)
                {
                    topics[i].publisher->Publish(to_payload(topics[i].sendIdx++));
                    std::promise<void> messagesDelivered;
                    auto future = messagesDelivered.get_future();
                    pubComAdapter->OnAllMessagesDelivered([&messagesDelivered]() {

                        messagesDelivered.set_value();

                    });
                    future.wait();
                }
            }
        }
    };

    // ------------------------------------------------------------
    // Wait until test is finished
    for (auto&& topic : topics)
    {
        auto testComplete = topic.testComplete.get_future();
        auto status = testComplete.wait_for(60s);
        ASSERT_EQ(status, std::future_status::ready);
        EXPECT_TRUE(testComplete.get());
    }
    
    publishThread.join();
}

} // anonymous namespace
