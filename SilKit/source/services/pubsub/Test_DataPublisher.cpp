// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataPublisher.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockParticipant.hpp"

#include "DataMessageDatatypeUtils.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

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

SilKit::Services::PubSub::PubSubSpec testDataNodeSpec{"Topic", {}};

class Test_DataPublisher : public ::testing::Test
{
protected:
    Test_DataPublisher()
        : publisher{&participant, participant.GetTimeProvider(), testDataNodeSpec, "pubUUID", {}}
    {
        publisher.SetServiceDescriptor(portAddress);
    }

protected:
    const ServiceDescriptor portAddress{"P1", "N1", "C1", 5};
    const std::vector<uint8_t> sampleData{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u};

    MockParticipant participant;
    DataPublisher publisher;
};

TEST_F(Test_DataPublisher, publish)
{
    WireDataMessageEvent msg{0ns, sampleData};

    EXPECT_CALL(participant, SendMsg(&publisher, msg)).Times(1);

    publisher.Publish(sampleData);
}

} // anonymous namespace
