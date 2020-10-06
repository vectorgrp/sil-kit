// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "ib/sim/all.hpp"

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

class SocketBufferSizeITest : public testing::Test
{
protected:
    SocketBufferSizeITest()
        : domainId{static_cast<uint32_t>(GetTestPid())}
    {
        topic.name = "TestTopic";

        ibConfig = ib::cfg::Config::FromJsonFile("SocketBufferSizeITest_IbConfig.json");
        
        pubComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Publisher");
        pubComAdapter->joinIbDomain(domainId);

        subComAdapter = CreateFastRtpsComAdapterImpl(ibConfig, "Subscriber");
        subComAdapter->joinIbDomain(domainId);
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

    std::unique_ptr<IComAdapterInternal> pubComAdapter;
    std::unique_ptr<IComAdapterInternal> subComAdapter;
};
    
TEST_F(SocketBufferSizeITest, DISABLED_use_custom_socket_buffer_size)
{
    Subscribe();

    // We cannot easily black-box test if the configured sendSocketBufferSize/listenSocketBufferSize is actually used by FastRTPS.
    // By transmitting some basic message we simply ensure that FastRTPS did not abort due to some bad XML format.
    // In addition, one can manually use a debugger to ensure if the values are read by FastRTPS into its internal runtime configuration data.
    std::vector<uint8_t> data(123, 'D');

    std::thread publishThread{[this, &data]() { this->Publish(data); }};

    auto&& reply = topic.reply.get_future();
    auto ready = reply.wait_for(5s);
    ASSERT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(reply.get(), data);
    
    publishThread.join();
}

} // anonymous namespace
