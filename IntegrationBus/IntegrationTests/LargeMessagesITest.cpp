// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>

#include "CreateComAdapter.hpp"
#include "ib/sim/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

#if IB_MW_HAVE_VASIO
#   include "VAsioRegistry.hpp"
#endif

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;

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

        registry = std::make_unique<VAsioRegistry>(ibConfig);
        registry->ProvideDomain(domainId);
        
        pubComAdapter = CreateComAdapterImpl(ibConfig, "Publisher");
        pubComAdapter->joinIbDomain(domainId);

        subComAdapter = CreateComAdapterImpl(ibConfig, "Subscriber");
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
    std::unique_ptr<VAsioRegistry> registry;
};
    
#if defined(IB_MW_HAVE_VASIO)
TEST_F(LargeMessagesITest, publish_and_subscribe_large_messages)
{
    Subscribe();

    size_t sizeInBytes = 114793;
    std::vector<uint8_t> data(sizeInBytes, 'D');

    std::thread publishThread{[this, &data]() { this->Publish(data); }};

    auto&& reply = topic.reply.get_future();
    auto ready = reply.wait_for(10min);
    ASSERT_EQ(ready, std::future_status::ready);
    EXPECT_EQ(reply.get(), data);
    
    publishThread.join();
}
#endif //IB_MW_HAVE_VASIO

} // anonymous namespace
