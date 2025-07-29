// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RegistryConfiguration.hpp"

#include "FileHelpers.hpp"

namespace {

void CheckFull(const SilKitRegistry::Config::V1::RegistryConfiguration& c)
{
    EXPECT_EQ(c.description, "Test_RegistryConfiguration_Full");

    ASSERT_TRUE(c.listenUri.has_value());
    EXPECT_EQ(c.listenUri.value(), "silkit://0.0.0.0:8501");

    ASSERT_TRUE(c.enableDomainSockets.has_value());
    EXPECT_EQ(c.enableDomainSockets.value(), false);

    ASSERT_EQ(c.logging.sinks.size(), 2u);
    EXPECT_EQ(c.logging.sinks[0].type, SilKit::Config::Sink::Type::Stdout);
    EXPECT_EQ(c.logging.sinks[0].level, SilKit::Services::Logging::Level::Trace);
    EXPECT_EQ(c.logging.sinks[1].type, SilKit::Config::Sink::Type::File);
    EXPECT_EQ(c.logging.sinks[1].level, SilKit::Services::Logging::Level::Debug);
    EXPECT_EQ(c.logging.sinks[1].logName, "FileSink");

    ASSERT_TRUE(c.dashboardUri.has_value());
    EXPECT_EQ(c.dashboardUri.value(), "http://dashboard.example.com:1234");
    EXPECT_TRUE(c.experimental.metrics.collectFromRemote);
}

TEST(Test_RegistryConfiguration, FullJson)
{
    const auto json = SilKit::Util::ReadTextFile("Test_RegistryConfiguration_Full.json");
    const auto config = SilKitRegistry::Config::Parse(json);
    CheckFull(config);
}

TEST(Test_RegistryConfiguration, FullYaml)
{
    const auto yaml = SilKit::Util::ReadTextFile("Test_RegistryConfiguration_Full.yaml");
    const auto config = SilKitRegistry::Config::Parse(yaml);
    CheckFull(config);
}


void CheckEmpty(const SilKitRegistry::Config::V1::RegistryConfiguration& c)
{
    EXPECT_EQ(c.description, "");

    ASSERT_FALSE(c.listenUri.has_value());
    ASSERT_FALSE(c.enableDomainSockets.has_value());
    ASSERT_EQ(c.logging.sinks.size(), 0u);
    ASSERT_FALSE(c.dashboardUri.has_value());
}

TEST(Test_RegistryConfiguration, EmptyJson)
{
    const auto json = std::string{"{}"};
    const auto config = SilKitRegistry::Config::Parse(json);
    CheckEmpty(config);
}

TEST(Test_RegistryConfiguration, EmptyYaml)
{
    const auto yaml = std::string{""};
    const auto config = SilKitRegistry::Config::Parse(yaml);
    CheckEmpty(config);
}

} // namespace
