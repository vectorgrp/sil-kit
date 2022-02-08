// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "DataMessageDatatypeUtils.hpp"

namespace {

using namespace testing;

using namespace ib::sim::data;

class DataExchangeFormatTest : public ::testing::Test
{
protected:
    DataExchangeFormatTest() {}
};

TEST_F(DataExchangeFormatTest, match_dataexchangeformat)
{
    DataExchangeFormat defPub{"A"};
    DataExchangeFormat defSub{"A"};
    EXPECT_EQ(Match(defSub, defPub), true); // same string, match

    defSub = {""};
    EXPECT_EQ(Match(defSub, defPub), true); // empty subscriber dxf = wildcard, match

    defSub = {"B"};
    EXPECT_EQ(Match(defSub, defPub), false); // different strings, no match

    defPub = {""}; 
    EXPECT_EQ(Match(defSub, defPub), false); // empty publisher dxf != wildcard, no match
}

TEST_F(DataExchangeFormatTest, match_labels)
{
    std::map<std::string, std::string> innerSet{}; 
    std::map<std::string, std::string> outerSet{{"KeyA", "ValA"},{"KeyB", "ValB"} ,{"KeyC", "ValC"} };
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Empty innerSet, match

    innerSet = {{"KeyA", "ValA"}}; 
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same key+value pair, match
    
    innerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"}, {"KeyC", "ValC"}};
    outerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"}, {"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true);  // Same 3 key+value pairs, match

    innerSet = {{"KeyC", ""}}; 
    outerSet = {{"KeyC", "ValC"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // Same key+empty value, match

    innerSet = {{"KeyA", "ValA"}}; 
    outerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"} };
    EXPECT_EQ(MatchLabels(innerSet, outerSet), true); // All labels of inner set in outer set, match

    innerSet = {{"KeyA", "ValA"}, {"KeyB", "ValB"} }; 
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // More labels in inner set than outer set, no match

    innerSet = {{"KeyA", "X"}}; 
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Wrong value, no match

    innerSet = {{"X", "ValA"}}; 
    outerSet = {{"KeyA", "ValA"}};
    EXPECT_EQ(MatchLabels(innerSet, outerSet), false); // Wrong key, no match
}


} // anonymous namespace
