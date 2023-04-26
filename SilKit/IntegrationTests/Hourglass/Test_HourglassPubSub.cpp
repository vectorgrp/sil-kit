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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ByteVectorMatcher.hpp"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/util/Span.hpp"

#include "MockCapiTest.hpp"

#include <algorithm>

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::StrEq;
using testing::Return;

using namespace SilKit::Services::PubSub;
using SilKit::Services::MatchingLabel;
using SilKit::Util::Span;

struct MatchingLabelEquals
{
    bool operator()(const MatchingLabel& lhs, const MatchingLabel& rhs) const
    {
        return std::make_tuple(lhs.key, lhs.value, lhs.kind) == std::make_tuple(rhs.key, rhs.value, rhs.kind);
    }
};

struct MatchingLabelLess
{
    bool operator()(const MatchingLabel& lhs, const MatchingLabel& rhs) const
    {
        return std::less<>{}(std::make_tuple(lhs.key, lhs.value, lhs.kind),
                             std::make_tuple(rhs.key, rhs.value, rhs.kind));
    }
};

MATCHER_P(PubSubSpecMatcher, pubSubSpecParam, "")
{
    const PubSubSpec& cppPubSubSpec = pubSubSpecParam;
    const SilKit_DataSpec* cDataSpec = arg;

    if (cDataSpec == nullptr)
    {
        *result_listener << "cDataSpec must not be nullptr";
        return false;
    }

    if (cDataSpec->topic == nullptr)
    {
        *result_listener << "cDataSpec->topic must not be nullptr";
        return false;
    }

    if (cDataSpec->mediaType == nullptr)
    {
        *result_listener << "cDataSpec->mediaType must not be nullptr";
        return false;
    }

    if (cppPubSubSpec.Topic() != cDataSpec->topic)
    {
        *result_listener << "topic does not match";
        return false;
    }

    if (cppPubSubSpec.MediaType() != cDataSpec->mediaType)
    {
        *result_listener << "mediaType does not match";
        return false;
    }

    if (cDataSpec->labelList.numLabels != cppPubSubSpec.Labels().size())
    {
        *result_listener << "number-of-labels does not match";
        return false;
    }

    using MatchingLabelSet = std::set<MatchingLabel, MatchingLabelLess>;

    MatchingLabelSet cppLabels{cppPubSubSpec.Labels().begin(), cppPubSubSpec.Labels().end()};

    MatchingLabelSet cLabels{};
    std::transform(cDataSpec->labelList.labels, cDataSpec->labelList.labels + cDataSpec->labelList.numLabels,
                   std::inserter(cLabels, cLabels.begin()), [](const SilKit_Label& cLabel) {
                       return MatchingLabel{cLabel.key, cLabel.value, static_cast<MatchingLabel::Kind>(cLabel.kind)};
                   });

    if (!std::equal(cppLabels.begin(), cppLabels.end(), cLabels.begin(), cLabels.end(), MatchingLabelEquals{}))
    {
        *result_listener << "labels do not match";
        return false;
    }

    return true;
}

class HourglassPubSubTest : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_DataPublisher* mockDataPublisher{reinterpret_cast<SilKit_DataPublisher*>(uintptr_t(0x78563412))};
    SilKit_DataSubscriber* mockDataSubscriber{reinterpret_cast<SilKit_DataSubscriber*>(uintptr_t(0x87654321))};

    HourglassPubSubTest()
    {
        using testing::_;
        ON_CALL(capi, SilKit_DataPublisher_Create(_, _, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockDataPublisher), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_DataSubscriber_Create(_, _, _, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockDataSubscriber), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

// DataPublisher

TEST_F(HourglassPubSubTest, SilKit_DataPublisher_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};

    std::string publisherName = "DataPublisher1";
    std::string topic = "Topic1";
    std::string mediaType = "MediaType1";
    std::string labelKey1 = "LabelKey1";
    std::string labelKey2 = "LabelKey2";
    std::string labelValue1 = "LabelValue1";
    std::string labelValue2 = "LabelValue2";

    PubSubSpec pubSubSpec{topic, mediaType};
    pubSubSpec.AddLabel(labelKey1, labelValue1, MatchingLabel::Kind::Mandatory);
    pubSubSpec.AddLabel(labelKey2, labelValue2, MatchingLabel::Kind::Optional);

    EXPECT_CALL(capi, SilKit_DataPublisher_Create(testing::_, participant, StrEq(publisherName),
                                                  PubSubSpecMatcher(pubSubSpec), 0x42));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::PubSub::DataPublisher publisher{
        participant, publisherName, pubSubSpec, 0x42};
}

TEST_F(HourglassPubSubTest, SilKit_DataPublisher_Publish)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::PubSub::DataPublisher publisher{
        participant, "DataPublisher1", PubSubSpec{"Topic1", "MediaType1"}, 0x42};

    std::vector<uint8_t> bytes{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const Span<uint8_t> byteSpan{bytes};

    EXPECT_CALL(capi, SilKit_DataPublisher_Publish(mockDataPublisher, ByteVectorMatcher(byteSpan)));

    publisher.Publish(byteSpan);
}

// DataSubscriber

TEST_F(HourglassPubSubTest, SilKit_DataSubscriber_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};

    std::string subscriberName = "DataSubscriber1";
    std::string topic = "Topic1";
    std::string mediaType = "MediaType1";
    std::string labelKey1 = "LabelKey1";
    std::string labelKey2 = "LabelKey2";
    std::string labelValue1 = "LabelValue1";
    std::string labelValue2 = "LabelValue2";

    PubSubSpec pubSubSpec{topic, mediaType};
    pubSubSpec.AddLabel(labelKey1, labelValue1, MatchingLabel::Kind::Mandatory);
    pubSubSpec.AddLabel(labelKey2, labelValue2, MatchingLabel::Kind::Optional);

    EXPECT_CALL(capi, SilKit_DataSubscriber_Create(testing::_, participant, StrEq(subscriberName),
                                                   PubSubSpecMatcher(pubSubSpec), testing::_, testing::_));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::PubSub::DataSubscriber subscriber{
        participant, subscriberName, pubSubSpec, [](IDataSubscriber*, const DataMessageEvent&) {
            // do nothing
        }};
}

TEST_F(HourglassPubSubTest, SilKit_DataSubscriber_SetDataMessageHandler)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::PubSub::DataSubscriber subscriber{
        participant, "DataSubscriber1", PubSubSpec{"Topic1", "MediaType1"},
        [](IDataSubscriber*, const DataMessageEvent&) {
            // do nothing
        }};

    EXPECT_CALL(capi, SilKit_DataSubscriber_SetDataMessageHandler(mockDataSubscriber, testing::_, testing::_));

    subscriber.SetDataMessageHandler([](IDataSubscriber*, const DataMessageEvent&) {
        // do nothing
    });
}

} //namespace
