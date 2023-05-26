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

    ASSERT_EQ(c.logging.sinks.size(), 2);
    EXPECT_EQ(c.logging.sinks[0].type, SilKit::Config::Sink::Type::Stdout);
    EXPECT_EQ(c.logging.sinks[0].level, SilKit::Services::Logging::Level::Trace);
    EXPECT_EQ(c.logging.sinks[1].type, SilKit::Config::Sink::Type::File);
    EXPECT_EQ(c.logging.sinks[1].level, SilKit::Services::Logging::Level::Debug);
    EXPECT_EQ(c.logging.sinks[1].logName, "FileSink");

    ASSERT_TRUE(c.dashboardUri.has_value());
    EXPECT_EQ(c.dashboardUri.value(), "http://dashboard.example.com:1234");
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
    ASSERT_EQ(c.logging.sinks.size(), 0);
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
