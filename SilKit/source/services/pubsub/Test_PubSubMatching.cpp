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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <unordered_set>

#include "DataMessageDatatypeUtils.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/participant/exception.hpp"

namespace {

using namespace testing;

using namespace SilKit::Util;
using namespace SilKit::Services;
using namespace SilKit::Services::PubSub;

class PubSubMatchingTest : public ::testing::Test
{
protected:
    PubSubMatchingTest() {}
};

TEST_F(PubSubMatchingTest, add_labels)
{
    PubSubSpec spec{"topic", "mediatype"};
    EXPECT_EQ(spec.Topic(), "topic");
    EXPECT_EQ(spec.MediaType(), "mediatype");
    EXPECT_TRUE(spec.Labels().empty());

    EXPECT_THROW(spec.AddLabel("key", "val", (MatchingLabel::Kind)0u), SilKit::ConfigurationError);

    MatchingLabel badLabel{};
    badLabel.key = "key";
    badLabel.value = "value";
    EXPECT_THROW(spec.AddLabel(badLabel), SilKit::ConfigurationError);

    MatchingLabel goodLabel{"key", "value", MatchingLabel::Kind::Mandatory};
    spec.AddLabel(goodLabel);
    EXPECT_EQ(spec.Labels()[0].key, "key");
    EXPECT_EQ(spec.Labels()[0].value, "value");
    EXPECT_EQ(spec.Labels()[0].kind, MatchingLabel::Kind::Mandatory);
}

TEST_F(PubSubMatchingTest, match_mediatype)
{
    std::string mediaTypePub{"A"};
    std::string mediaTypeSub{"A"};
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), true); // Same string, match

    mediaTypeSub = "";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), true); // Empty subscriber mediaType = wildcard, match

    mediaTypeSub = "B";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), false); // Different strings, no match

    mediaTypePub = "";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), false); // Empty publisher mediaType != wildcard, no match
}

} // anonymous namespace
