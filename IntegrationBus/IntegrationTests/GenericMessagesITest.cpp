// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"
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
        : domainId{static_cast<uint32_t>(GetTestPid())}
        , topics(2)
    {
        topics[0].name = "GroundTruth";
        topics[1].name = "VehicleModelOut";

        ibConfig = ib::cfg::Config::FromJsonFile("GenericMessagesITest_IbConfig.json");
    }

    void Subscribe()
    {
        subComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Subscriber");
        subComAdapter->joinIbDomain(domainId);

        for (auto&& topic: topics)
        {
            auto subscriber = subComAdapter->CreateGenericSubscriber(topic.name);

            subscriber->SetReceiveMessageHandler(
                [&topic = topic, this](auto* /*subscriber*/, auto&& data)
                {
                    topic.reply = std::string{data.begin(), data.end()};

                    if (topics.size() == ++receiveCount)
                        allReceivedPromise.set_value();
                }
            );
        }
    }

    void Publish()
    {
        pubComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Publisher");
        pubComAdapter->joinIbDomain(domainId);

        for (auto&& topic: topics)
        {
            auto publisher = pubComAdapter->CreateGenericPublisher(topic.name);

            publisher->Publish({topic.name.begin(), topic.name.end()});
        }
    }

    struct Topic
    {
        std::string name;
        std::string reply;
    };

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::vector<Topic> topics;

    unsigned int receiveCount{0};
    std::promise<void> allReceivedPromise;

    std::unique_ptr<ComAdapter<FastRtpsConnection>> pubComAdapter;
    std::unique_ptr<ComAdapter<FastRtpsConnection>> subComAdapter;
};
    
TEST_F(GenericMessageITest, publish_and_subscribe_generic_messages)
{
    std::thread subscribeThread{[this] { Subscribe(); }};
    std::thread publishThread{[this]{ Publish(); }};

    auto allReceived = allReceivedPromise.get_future();
    allReceived.wait_for(10min);

    publishThread.join();
    subscribeThread.join();

    for (auto&& topic : topics)
    {
        EXPECT_EQ(topic.name, topic.reply);
    }
}

} // anonymous namespace
