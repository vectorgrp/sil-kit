// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "StringHelpers.hpp"


namespace {

const std::string content{R"(This is the " sample content for the StringHelpers \ StringEscape tests. )"};
const std::string result{R"(This is the \" sample content for the StringHelpers \\ StringEscape tests. )"};


TEST(Test_Util_StringHelpers, EscapeString)
{
    ASSERT_EQ(result, SilKit::Util::EscapeString(content));
}

} // anonymous namespace
