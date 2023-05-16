/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

class DataPublisherTest : public ::testing::Test
{
protected:
    DataPublisherTest()
        : publisher{ &participant, participant.GetTimeProvider(), testDataNodeSpec, "pubUUID", {} }
    {
        publisher.SetServiceDescriptor(portAddress);
    }

protected:
    const ServiceDescriptor portAddress{ "P1", "N1", "C1", 5};
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
