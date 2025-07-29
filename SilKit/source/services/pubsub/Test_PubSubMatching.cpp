// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

class Test_PubSubMatching : public ::testing::Test
{
protected:
    Test_PubSubMatching() {}
};

TEST_F(Test_PubSubMatching, add_labels)
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

TEST_F(Test_PubSubMatching, match_mediatype)
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
