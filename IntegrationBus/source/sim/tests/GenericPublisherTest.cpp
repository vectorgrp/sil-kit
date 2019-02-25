// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "GenericPublisher.hpp"

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

class GenericPublisherTest : public ::testing::Test
{
protected:
    GenericPublisherTest()
        : publisher{&comAdapter, config}
    {
        publisher.SetEndpointAddress(portAddress);
    }

    // Workaround for MS VS2015 where we cannot intialize the config member directly
    static auto MakeConfig() -> cfg::GenericPort
    {
        cfg::GenericPort config;

        config.name = "Publisher";
        config.endpointId = 5;
        config.linkId = 3;
        config.protocolType = cfg::GenericPort::ProtocolType::ROS;
        config.definitionUri = "file://SubscriberMessage.msg";

        return config;
    }
protected:
    const cfg::GenericPort config{MakeConfig()};
    const EndpointAddress portAddress{4, 5};
    const std::vector<uint8_t> sampleData{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    test::MockComAdapter comAdapter;
    GenericPublisher publisher;
};

TEST_F(GenericPublisherTest, publish_vector)
{
    const GenericMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(portAddress, msg))
        .Times(1);

    publisher.Publish(sampleData);
}

TEST_F(GenericPublisherTest, publish_raw)
{
    const GenericMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(portAddress, msg))
        .Times(1);

    publisher.Publish(sampleData.data(), sampleData.size());
}

TEST_F(GenericPublisherTest, get_name_from_publisher)
{
    EXPECT_EQ(publisher.Config().name, config.name);
    EXPECT_EQ(publisher.Config(), config);
}


} // anonymous namespace
