// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataPublisher.hpp"

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

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    void SendIbMessage(const IIbServiceEndpoint* from, DataMessage&& msg) override
    {
        SendIbMessage_proxy(from, msg);
    }

    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const DataMessage&));
    MOCK_METHOD2(SendIbMessage_proxy, void(const IIbServiceEndpoint*, const DataMessage&));
};

class DataPublisherTest : public ::testing::Test
{
protected:
    DataPublisherTest()
        : publisher{ &comAdapter, comAdapter.GetTimeProvider(), "Topic", DataExchangeFormat{}, {}, "pubUUID", 0 }
    {
        publisher.SetServiceDescriptor(from_endpointAddress(portAddress));
    }

protected:
    const EndpointAddress portAddress{4, 5};
    const std::vector<uint8_t> sampleData{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    MockComAdapter comAdapter;
    DataPublisher publisher;
};

TEST_F(DataPublisherTest, publish_vector)
{
    const DataMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(&publisher, msg))
        .Times(1);

    publisher.Publish(sampleData);
}

TEST_F(DataPublisherTest, publish_raw)
{
    const DataMessage msg{sampleData};

    EXPECT_CALL(comAdapter, SendIbMessage_proxy(&publisher, msg))
        .Times(1);

    publisher.Publish(sampleData.data(), sampleData.size());
}

} // anonymous namespace
