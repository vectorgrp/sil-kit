// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <unordered_set>

#include "RpcDatatypeUtils.hpp"

namespace {

using namespace testing;

using namespace SilKit::Services::Rpc;

class RpcMatchingTest : public ::testing::Test
{
protected:
    RpcMatchingTest() {}
};

TEST_F(RpcMatchingTest, match_mediatype)
{
    std::string mediaTypePub{"A"};
    std::string mediaTypeSub{"A"};
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), true); // same string, match

    mediaTypeSub = "";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), true); // empty subscriber mediaType = wildcard, match

    mediaTypeSub = "B";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), false); // different strings, no match

    mediaTypePub = "";
    EXPECT_EQ(MatchMediaType(mediaTypeSub, mediaTypePub), false); // empty publisher mediaType != wildcard, no match
}

TEST_F(RpcMatchingTest, match_labels_mandatory)
{
    std::vector<SilKit::Services::MatchingLabel> innerSet{};
    std::vector<SilKit::Services::Label> outerSet{
        {"KeyA", "ValA"},
        {"KeyB", "ValB"},
        {"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Empty innerSet, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same key+value pair, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory},
                {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Mandatory},
                {"KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"},
                {"KeyB", "ValB"},
                {"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same 3 key+value pairs, match

    innerSet = {{"KeyC", "", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Same key+empty value, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"},
                {"KeyB", "ValB"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // All labels of inner set in outer set, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory},
                {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // More labels in inner set than outer set, no match

    innerSet = {{"KeyA", "X", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Wrong value, no match

    innerSet = {{"X", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Wrong key, no match
}

TEST_F(RpcMatchingTest, match_labels_preferred)
{
    std::vector<SilKit::Services::MatchingLabel> innerSet{};
    std::vector<SilKit::Services::Label> outerSet{{"KeyA", "ValA"}, {"KeyB", "ValB"}, {"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Empty innerSet, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same key+value pair, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred},
                {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Preferred},
                {"KeyC", "ValC", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"}, {"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same 3 key+value pairs, match

    innerSet = {{"KeyC", "", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Same key+empty value, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // All labels of inner set in outer set, match

    innerSet = {{"KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred},
                {"KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // More labels in inner set than outer set, no match

    innerSet = {{"KeyA", "X", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Wrong value, no match

    innerSet = {{"X", "ValA", SilKit::Services::MatchingLabel::Kind::Preferred}};
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Wrong key,  match
}

} // anonymous namespace
