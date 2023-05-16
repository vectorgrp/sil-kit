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
        subscriber.SetServiceDescriptor(endpointAddress);
        subscriber.SetDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));

        subscriberOther.SetServiceDescriptor(otherEndpointAddress);
        subscriberOther.SetDataMessageHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::ReceiveDataDefault));
    }

protected:
    const ServiceDescriptor endpointAddress{ "P1", "N1", "C1" , 5};
    const ServiceDescriptor otherEndpointAddress{ "P2", "N1", "C3", 7 };

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
} // anonymous namespace
