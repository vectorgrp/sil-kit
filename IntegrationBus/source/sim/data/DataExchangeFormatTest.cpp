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

TEST_F(DataExchangeFormatTest, wildcard_match)
{
    const DataExchangeFormat defA{"A"};
    const DataExchangeFormat defMatchA{"A"};
    const DataExchangeFormat defMatchAWildcard{"*"};
    const DataExchangeFormat defNoMatchA{"B"};

    EXPECT_EQ(Match(defA, defMatchA), true);
    EXPECT_EQ(Match(defA, defMatchAWildcard), true);
    EXPECT_EQ(Match(defA, defNoMatchA), false);
}

TEST_F(DataExchangeFormatTest, join_dataexchangeformat)
{
    const DataExchangeFormat defA{"A"};
    const DataExchangeFormat defB{"*"};
    const DataExchangeFormat defJoined{"A"};

    EXPECT_EQ(Match(defA, defB), true);
    EXPECT_EQ(Join(defA, defB), defJoined);
}

} // anonymous namespace
