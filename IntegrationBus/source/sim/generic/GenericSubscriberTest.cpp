// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericSubscriber.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"

#include "GenericMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::sim::generic;

class GenericSubscriberTest : public ::testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveData, void(IGenericSubscriber*, const std::vector<uint8_t>& data));
    };

protected:
    GenericSubscriberTest()
        : subscriber{&comAdapter, config, comAdapter.GetTimeProvider()}
    {
        subscriber.SetEndpointAddress(endpointAddress);
        subscriber.SetReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveData));
    }

    // Workaround for MS VS2015 where we cannot intialize the config member directly
    static auto MakeConfig() -> cfg::GenericPort
    {
        cfg::GenericPort config;

        config.name = "Subscriber";
        config.endpointId = 5;
        config.linkId = 3;
        config.protocolType = cfg::GenericPort::ProtocolType::ROS;
        config.definitionUri = "file://SubscriberMessage.msg";

        return config;
    }

protected:
    const cfg::GenericPort config{MakeConfig()};
    const EndpointAddress endpointAddress{4, 5};
    const EndpointAddress otherEndpointAddress{5, 7};

    test::DummyComAdapter comAdapter;
    Callbacks callbacks;
    GenericSubscriber subscriber;
};

TEST_F(GenericSubscriberTest, trigger_callback)
{
    const GenericMessage msg{{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveData(&subscriber, msg.data))
        .Times(1);

    subscriber.ReceiveIbMessage(otherEndpointAddress, msg);
}

TEST_F(GenericSubscriberTest, get_name_from_subscriber)
{
    EXPECT_EQ(subscriber.Config().name, config.name);
    EXPECT_EQ(subscriber.Config(), config);
}

} // anonymous namespace
