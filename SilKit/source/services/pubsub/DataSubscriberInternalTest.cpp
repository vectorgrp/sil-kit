// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/util/functional.hpp"

#include "MockParticipant.hpp"

#include "DataMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::PubSub;

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
        subscriber.SetDefaultDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));

        subscriberOther.SetServiceDescriptor(from_endpointAddress(otherEndpointAddress));
        subscriberOther.SetDefaultDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));
    }

protected:
    const EndpointAddress endpointAddress{4, 5};
    const EndpointAddress otherEndpointAddress{5, 7};

    SilKit::Core::Tests::DummyParticipant participant;
    Callbacks callbacks;
    DataSubscriberInternal subscriber;
    DataSubscriberInternal subscriberOther;
};

TEST_F(DataSubscriberInternalTest, trigger_default_data_handler)
{
    const WireDataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveDataDefault(nullptr, ToDataMessageEvent(msg))).Times(1);

    subscriber.ReceiveMsg(&subscriberOther, msg);
}

TEST_F(DataSubscriberInternalTest, trigger_explicit_data_handler_fallback_default_data_handler)
{
    const WireDataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    const auto handlerId =
        subscriber.AddExplicitDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataExplicit));

    EXPECT_CALL(callbacks, ReceiveDataDefault(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, ReceiveDataExplicit(nullptr, ToDataMessageEvent(msg))).Times(1);
    subscriber.ReceiveMsg(&subscriberOther, msg);

    subscriber.RemoveExplicitDataMessageHandler(handlerId);

    EXPECT_CALL(callbacks, ReceiveDataDefault(nullptr, ToDataMessageEvent(msg))).Times(1);
    EXPECT_CALL(callbacks, ReceiveDataExplicit(testing::_, testing::_)).Times(0);
    subscriber.ReceiveMsg(&subscriberOther, msg);
}

} // anonymous namespace
