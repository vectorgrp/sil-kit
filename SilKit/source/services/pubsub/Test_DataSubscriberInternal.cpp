// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataSubscriberInternal.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "functional.hpp"

#include "MockParticipant.hpp"

#include "DataMessageDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::PubSub;

class Test_DataSubscriberInternal : public ::testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD2(ReceiveDataDefault, void(IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
        MOCK_METHOD2(ReceiveDataExplicit, void(IDataSubscriber*, const DataMessageEvent& dataMessageEvent));
    };

protected:
    Test_DataSubscriberInternal()
        : subscriber{&participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr}
        , subscriberOther{&participant, participant.GetTimeProvider(), "Topic", {}, {}, {}, nullptr}
    {
        subscriber.SetServiceDescriptor(endpointAddress);
        subscriber.SetDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));

        subscriberOther.SetServiceDescriptor(otherEndpointAddress);
        subscriberOther.SetDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));
    }

protected:
    const ServiceDescriptor endpointAddress{"P1", "N1", "C1", 5};
    const ServiceDescriptor otherEndpointAddress{"P2", "N1", "C3", 7};

    SilKit::Core::Tests::DummyParticipant participant;
    Callbacks callbacks;
    DataSubscriberInternal subscriber;
    DataSubscriberInternal subscriberOther;
};

TEST_F(Test_DataSubscriberInternal, trigger_default_data_handler)
{
    const WireDataMessageEvent msg{0ns, {0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u}};

    EXPECT_CALL(callbacks, ReceiveDataDefault(nullptr, ToDataMessageEvent(msg))).Times(1);

    subscriber.ReceiveMsg(&subscriberOther, msg);
}
} // anonymous namespace
