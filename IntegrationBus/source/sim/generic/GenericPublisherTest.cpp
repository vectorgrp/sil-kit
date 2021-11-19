// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GenericPublisher.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "MockComAdapter.hpp"
#include "MockTraceSink.hpp"

#include "GenericMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::sim::generic;

using ::ib::mw::test::DummyComAdapter;
using ::ib::test::MockTraceSink;

class MockComAdapter : public DummyComAdapter
{
public:
    void SendIbMessage(const IServiceId* from, GenericMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD2(SendIbMessage, void(IServiceId*, const GenericMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(const IServiceId*, const GenericMessage&));
};

class GenericPublisherTest : public ::testing::Test
{
protected:
    GenericPublisherTest()
        : publisher{&comAdapter, config, comAdapter.GetTimeProvider()}
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

    MockComAdapter comAdapter;
    MockTraceSink traceSink;
    GenericPublisher publisher;
};

TEST_F(GenericPublisherTest, publish_vector)
{
    const GenericMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(&publisher, msg))
        .Times(1);

    publisher.Publish(sampleData);
}

TEST_F(GenericPublisherTest, publish_raw)
{
    const GenericMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(&publisher, msg))
        .Times(1);

    publisher.Publish(sampleData.data(), sampleData.size());
}

TEST_F(GenericPublisherTest, get_name_from_publisher)
{
    EXPECT_EQ(publisher.Config().name, config.name);
    EXPECT_EQ(publisher.Config(), config);
}


TEST_F(GenericPublisherTest, publish_with_tracing)
{
    using namespace ib::extensions;

    const auto now = 1337ns;
    ON_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .WillByDefault(testing::Return(now));

    publisher.AddSink(&traceSink);
    const GenericMessage msg{sampleData};

    EXPECT_CALL(comAdapter.mockTimeProvider.mockTime, Now())
        .Times(1);
    EXPECT_CALL(traceSink,
        Trace(Direction::Send, portAddress, now, msg))
        .Times(1);

    publisher.Publish(sampleData);
}
} // anonymous namespace
