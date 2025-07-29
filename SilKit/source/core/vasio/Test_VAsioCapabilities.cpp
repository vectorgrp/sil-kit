// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "silkit/participant/exception.hpp"

#include "VAsioCapabilities.hpp"


namespace {

TEST(Test_VAsioCapabilities, parse_empty_capabilities)
{
    EXPECT_EQ(SilKit::Core::VAsioCapabilities{}.Count(), 0u);
    EXPECT_EQ(SilKit::Core::VAsioCapabilities{""}.Count(), 0u);
    EXPECT_EQ(SilKit::Core::VAsioCapabilities{"[]"}.Count(), 0u);
}

TEST(Test_VAsioCapabilities, parse_single_capability)
{
    const auto capabilities = SilKit::Core::VAsioCapabilities{R"([{"name":"abc"}])"};

    EXPECT_EQ(capabilities.Count(), 1u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));

    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));
}

TEST(Test_VAsioCapabilities, parse_single_capability_with_duplicates)
{
    const auto capabilities = SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},{"name":"abc"}])"};

    EXPECT_EQ(capabilities.Count(), 1u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));

    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));
}

TEST(Test_VAsioCapabilities, parse_multiple_capabilities_without_duplicates)
{
    const auto capabilities = SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},{"name":"def"},{"name":"ghi"}])"};

    EXPECT_EQ(capabilities.Count(), 3u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_TRUE(capabilities.HasCapability("def"));
    EXPECT_TRUE(capabilities.HasCapability("ghi"));

    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));
}

TEST(Test_VAsioCapabilities, parse_multiple_capabilities_with_duplicates)
{
    const auto capabilities = SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},{"name":"def"},{"name":"abc"}])"};

    EXPECT_EQ(capabilities.Count(), 2u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_TRUE(capabilities.HasCapability("def"));

    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));
}

TEST(Test_VAsioCapabilities, permit_and_ignore_additional_field)
{
    const auto capabilities = SilKit::Core::VAsioCapabilities{R"([{"name":"abc","a":"b","c":"d"}])"};

    EXPECT_EQ(capabilities.Count(), 1u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
}

TEST(Test_VAsioCapabilities, throw_invalid_top_level)
{
    // non-empty, but just whitespace is not allowed
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"( )"}, SilKit::SilKitError);
    // top-level is not a sequence
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"(some-characters)"}, SilKit::SilKitError);
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"("top-level-string")"}, SilKit::SilKitError);
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"({"a":"b","c":"d"})"}, SilKit::SilKitError);
    // top-level is a sequence, but contains not just maps
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"([some-characters])"}, SilKit::SilKitError);
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"(["item-string"])"}, SilKit::SilKitError);
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},"item-string"])"}, SilKit::SilKitError);
    // top-level is a sequence, contains just maps, but a map has no name key
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},{}])"}, SilKit::SilKitError);
    EXPECT_THROW(SilKit::Core::VAsioCapabilities{R"([{"name":"abc"},{"woop":"hoop"}])"}, SilKit::SilKitError);
}

TEST(Test_VAsioCapabilities, parse_add_capability)
{
    SilKit::Core::VAsioCapabilities capabilities;

    capabilities.AddCapability("abc");
    EXPECT_EQ(capabilities.Count(), 1u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_FALSE(capabilities.HasCapability("def"));
    EXPECT_FALSE(capabilities.HasCapability("ghi"));
    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));

    capabilities.AddCapability("def");
    EXPECT_EQ(capabilities.Count(), 2u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_TRUE(capabilities.HasCapability("def"));
    EXPECT_FALSE(capabilities.HasCapability("ghi"));
    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));

    capabilities.AddCapability("abc");
    EXPECT_EQ(capabilities.Count(), 2u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_TRUE(capabilities.HasCapability("def"));
    EXPECT_FALSE(capabilities.HasCapability("ghi"));
    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));

    capabilities.AddCapability("ghi");
    EXPECT_EQ(capabilities.Count(), 3u);
    EXPECT_TRUE(capabilities.HasCapability("abc"));
    EXPECT_TRUE(capabilities.HasCapability("def"));
    EXPECT_TRUE(capabilities.HasCapability("ghi"));
    EXPECT_FALSE(capabilities.HasCapability(""));
    EXPECT_FALSE(capabilities.HasCapability("xyz"));
    EXPECT_FALSE(capabilities.HasCapability("Hello World"));
}

TEST(Test_VAsioCapabilities, empty_roundtrip)
{
    const auto original = SilKit::Core::VAsioCapabilities{};
    const auto originalString = original.ToCapabilitiesString();

    // for compatibility reasons, the string transformation for an empty set should return an empty string
    EXPECT_EQ(originalString, "");

    const auto roundtrip = SilKit::Core::VAsioCapabilities{originalString};
    EXPECT_EQ(roundtrip.Count(), 0u);
}

TEST(Test_VAsioCapabilities, single_capability_roundtrip)
{
    auto original = SilKit::Core::VAsioCapabilities{};
    original.AddCapability("abc");

    const auto originalString = original.ToCapabilitiesString();

    // for compatibility reasons, the string transformation for an empty set should return an empty string
    EXPECT_NE(originalString, "");

    const auto roundtrip = SilKit::Core::VAsioCapabilities{originalString};
    EXPECT_EQ(roundtrip.Count(), 1u);
    EXPECT_TRUE(roundtrip.HasCapability("abc"));
}

TEST(Test_VAsioCapabilities, multiple_capabilities_roundtrip)
{
    auto original = SilKit::Core::VAsioCapabilities{};
    original.AddCapability("abc");
    original.AddCapability("def");
    original.AddCapability("ghi");

    const auto originalString = original.ToCapabilitiesString();

    // for compatibility reasons, the string transformation for an empty set should return an empty string
    EXPECT_NE(originalString, "");

    const auto roundtrip = SilKit::Core::VAsioCapabilities{originalString};
    EXPECT_EQ(roundtrip.Count(), 3u);
    EXPECT_TRUE(roundtrip.HasCapability("abc"));
    EXPECT_TRUE(roundtrip.HasCapability("def"));
    EXPECT_TRUE(roundtrip.HasCapability("ghi"));
}

} // namespace
