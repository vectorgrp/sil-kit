// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataPublisher.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockParticipant.hpp"

#include "DataMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::PubSub;

using ::SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const WireDataMessageEvent&), (override));
};

class DataPublisherTest : public ::testing::Test
{
protected:
    DataPublisherTest()
        : publisher{ &participant, participant.GetTimeProvider(), "Topic", {}, {}, "pubUUID" }
    {
        publisher.SetServiceDescriptor(from_endpointAddress(portAddress));
    }

protected:
    const EndpointAddress portAddress{4, 5};
    const std::vector<uint8_t> sampleData{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    MockParticipant participant;
    DataPublisher publisher;
};

TEST_F(DataPublisherTest, publish)
{
    WireDataMessageEvent msg{0ns, sampleData};

    EXPECT_CALL(participant, SendMsg(&publisher, msg))
        .Times(1);

    publisher.Publish(sampleData);
}

} // anonymous namespace
