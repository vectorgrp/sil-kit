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

class DataSubscriberInternalTest : public ::testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveDataDefault, void(IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
        MOCK_METHOD2(ReceiveDataExplicit, void(IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
    };

protected:
    DataSubscriberInternalTest()
        : subscriber{&participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr}
        , subscriberOther{&participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr}
    {
        subscriber.SetServiceDescriptor(from_endpointAddress(endpointAddress));
        subscriber.SetDefaultDataMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));

        subscriberOther.SetServiceDescriptor(from_endpointAddress(otherEndpointAddress));
        subscriberOther.SetDefaultDataMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));
    }

protected:
    const EndpointAddress endpointAddress{4, 5};
    const EndpointAddress otherEndpointAddress{5, 7};

    ib::mw::test::DummyParticipant participant;
    Callbacks callbacks;
    DataSubscriberInternal subscriber;
    DataSubscriberInternal subscriberOther;
};

TEST_F(DataSubscriberInternalTest, trigger_default_data_handler)
{
    const DataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveDataDefault(nullptr, msg)).Times(1);

    subscriber.ReceiveIbMessage(&subscriberOther, msg);
}

TEST_F(DataSubscriberInternalTest, trigger_explicit_data_handler_fallback_default_data_handler)
{
    const DataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    const auto handlerId =
        subscriber.AddExplicitDataMessageHandler(ib::util::bind_method(&callbacks, &Callbacks::ReceiveDataExplicit));

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicit(nullptr, msg)).Times(1);
    subscriber.ReceiveIbMessage(&subscriberOther, msg);

    subscriber.RemoveExplicitDataMessageHandler(handlerId);

    EXPECT_CALL(callbacks, ReceiveDataDefault(nullptr, msg)).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicit(testing::_, testing::_)).Times(0);
    subscriber.ReceiveIbMessage(&subscriberOther, msg);
}

} // anonymous namespace
