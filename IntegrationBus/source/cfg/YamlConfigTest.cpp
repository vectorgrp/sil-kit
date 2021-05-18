// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

class YamlConfigTest : public testing::Test
{
};

using namespace ib::cfg;
TEST_F(YamlConfigTest, yaml_doc_relations)
{
    //validate that YAML validation of config blocks works
    YamlValidator schema;
    EXPECT_TRUE(schema.IsTopLevelElement("ConfigVersion"));
    EXPECT_TRUE(schema.IsTopLevelElement("ConfigName"));
    EXPECT_TRUE(schema.IsTopLevelElement("Description"));
    EXPECT_TRUE(schema.IsTopLevelElement("SimulationSetup"));
    EXPECT_TRUE(schema.IsTopLevelElement("MiddlewareConfig"));
    EXPECT_TRUE(schema.IsTopLevelElement("LaunchConfigurations"));
    // simulation setup members
    EXPECT_TRUE(schema.IsChildOf( "Participants", "SimulationSetup"));
    EXPECT_TRUE(schema.IsChildOf("Switches", "SimulationSetup"));
    EXPECT_TRUE(schema.IsChildOf( "Links", "SimulationSetup"));
    EXPECT_TRUE(schema.IsChildOf( "NetworkSimulators", "SimulationSetup"));
    EXPECT_TRUE(schema.IsChildOf( "TimeSync", "SimulationSetup"));
}
} // anonymous namespace
