// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"

#include "DataMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::sim::data;

class DataSubscriberTest : public ::testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD3(ReceiveData, void(IDataSubscriber*, const std::vector<uint8_t>& data,
                                       const DataExchangeFormat& dataExchangeFormat));
    };

protected:
    DataSubscriberTest()
        : subscriber{ &comAdapter, config, comAdapter.GetTimeProvider(), {} }
        , subscriberOther{ &comAdapter, config, comAdapter.GetTimeProvider(), {} }
    {
        subscriber.SetServiceDescriptor(from_endpointAddress(endpointAddress));
        subscriber.SetReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveData));

        subscriberOther.SetServiceDescriptor(from_endpointAddress(otherEndpointAddress));
        subscriberOther.SetReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveData));
    }

    // Workaround for MS VS2015 where we cannot initialize the config member directly
    static auto MakeConfig() -> cfg::DataPort
    {
        cfg::DataPort config;

        config.name = "Subscriber";
        config.endpointId = 5;
        config.linkId = 3;

        return config;
    }

protected:
    const cfg::DataPort config{MakeConfig()};
    const EndpointAddress endpointAddress{4, 5};
    const EndpointAddress otherEndpointAddress{5, 7};

    ib::mw::test::DummyComAdapter comAdapter;
    Callbacks                     callbacks;
    DataSubscriberInternal        subscriber;
    DataSubscriberInternal        subscriberOther;
};

TEST_F(DataSubscriberTest, trigger_callback)
{
    const DataMessage msg{{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveData(&subscriber, msg.data, DataExchangeFormat{}))
        .Times(1);

    subscriber.ReceiveIbMessage(&subscriberOther, msg);
}

TEST_F(DataSubscriberTest, get_name_from_subscriber)
{
    EXPECT_EQ(subscriber.Config().name, config.name);
    EXPECT_EQ(subscriber.Config(), config);
}


} // anonymous namespace
