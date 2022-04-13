// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "MockParticipant.hpp"

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
        MOCK_METHOD2(ReceiveData, void(IDataSubscriber*, const std::vector<uint8_t>& data));
    };

protected:
    DataSubscriberTest()
        : subscriber{ &participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr }
        , subscriberOther{ &participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr }
    {
        subscriber.SetServiceDescriptor(from_endpointAddress(endpointAddress));
        subscriber.SetDefaultReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveData));

        subscriberOther.SetServiceDescriptor(from_endpointAddress(otherEndpointAddress));
        subscriberOther.SetDefaultReceiveMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveData));
    }


protected:
    const EndpointAddress endpointAddress{4, 5};
    const EndpointAddress otherEndpointAddress{5, 7};

    ib::mw::test::DummyParticipant participant;
    Callbacks                     callbacks;
    DataSubscriberInternal        subscriber;
    DataSubscriberInternal        subscriberOther;
};

TEST_F(DataSubscriberTest, trigger_callback)
{
    const DataMessage msg{{0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveData(nullptr, msg.data))
        .Times(1);

    subscriber.ReceiveIbMessage(&subscriberOther, msg);
}


} // anonymous namespace
