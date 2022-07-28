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

namespace {

using namespace testing;

using namespace SilKit::Services::PubSub;

class PubSubMatchingTest : public ::testing::Test
{
protected:
    PubSubMatchingTest() {}
};

TEST_F(PubSubMatchingTest, match_mediatype)
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

TEST_F(PubSubMatchingTest, match_labels_mandatory)
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

TEST_F(PubSubMatchingTest, match_labels_preferred)
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

TEST_F(PubSubMatchingTest, announced_sources_same_labels_different_order)
{
    std::map<std::string, std::string> labels1{{"K1", "V1"}, {"K2", "V2"}};
    std::map<std::string, std::string> labels2{{"K2", "V2"}, {"K1", "V1"}};
    EXPECT_EQ(labels1, labels2);

    std::unordered_set<SourceInfo, SourceInfo::HashFunction> announced;
    announced.emplace(SourceInfo{"M", labels1});
    announced.emplace(SourceInfo{"M", labels2}); // Is same, no emplace
    EXPECT_EQ(announced.size(), 1);
}

TEST_F(PubSubMatchingTest, announced_sources_different_labels)
{
    std::map<std::string, std::string> labels1{{"K1", "V1"}};
    std::map<std::string, std::string> labels2{{"K2", "V2"}};

    std::unordered_set<SourceInfo, SourceInfo::HashFunction> announced;
    announced.emplace(SourceInfo{"M", labels1});
    announced.emplace(SourceInfo{"M", labels2});
    EXPECT_EQ(announced.size(), 2);
}

TEST_F(PubSubMatchingTest, announced_sources_empty_labels)
{
    std::map<std::string, std::string> labels1{};
    std::map<std::string, std::string> labels2{};

    std::unordered_set<SourceInfo, SourceInfo::HashFunction> announced;
    announced.emplace(SourceInfo{"M", labels1});
    announced.emplace(SourceInfo{"M", labels2}); // Is same, no emplace
    EXPECT_EQ(announced.size(), 1);
}

TEST_F(PubSubMatchingTest, announced_sources_different_mediaype)
{
    std::map<std::string, std::string> labels1{{"K1", "V1"}};
    std::map<std::string, std::string> labels2{{"K1", "V1"}};

    std::unordered_set<SourceInfo, SourceInfo::HashFunction> announced;
    announced.emplace(SourceInfo{"M1", labels1});
    announced.emplace(SourceInfo{"M2", labels2});
    EXPECT_EQ(announced.size(), 2);
}


} // anonymous namespace
