// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "RpcDatatypeUtils.hpp"

namespace {

using namespace testing;

using namespace SilKit::Services::Rpc;

class Test_RpcMatching : public ::testing::Test
{
protected:
    Test_RpcMatching() {}
};

TEST_F(Test_RpcMatching, match_mediatype)
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

} // anonymous namespace
