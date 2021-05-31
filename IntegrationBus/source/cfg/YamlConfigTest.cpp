// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"
#include "YamlValidator.hpp"

#include "ib/cfg/Config.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

class YamlConfigTest : public testing::Test
{
};

using namespace ib::cfg;
TEST_F(YamlConfigTest, yaml_doc_relations)
{
    YamlValidator v;
    //ensure that YAML validation of config blocks works
    EXPECT_TRUE(v.IsRootElement("/ConfigVersion"));
    EXPECT_TRUE(v.IsRootElement("/ConfigName"));
    EXPECT_TRUE(v.IsRootElement("/Description"));
    EXPECT_TRUE(v.IsRootElement("/SimulationSetup"));
    EXPECT_TRUE(v.IsRootElement("/MiddlewareConfig"));
    EXPECT_TRUE(v.IsRootElement("/LaunchConfigurations"));

    EXPECT_FALSE(v.IsRootElement(" /LaunchConfigurations"));
    EXPECT_FALSE(v.IsRootElement("/SimulationSetup/Participants"));

    // simulation setup members
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Participants"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Switches"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/Links"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/NetworkSimulators"));
    EXPECT_TRUE(v.IsSubelementOf("/SimulationSetup", "/SimulationSetup/TimeSync"));

    EXPECT_FALSE(v.IsSubelementOf("/SimulationSetup", " /SimulationSetup/TimeSync"));
}
TEST_F(YamlConfigTest, candemo_in_yaml_format)
{
    auto demoYaml= R"raw(
ConfigName: CanDemo
ConfigVersion: 0.0.1
Description: Sample configuration for CAN without NetworkSimulator
SimulationSetup:
  Links:
  - Endpoints:
    - CanWriter/CAN1
    - CanReader/CAN1
    Name: CAN1
  Participants:
  - CanControllers:
    # supports comments now
    - Name: CAN1@CanWriter
      UseTraceSinks:
      - Sink1
    Description: Demo Writer
    Logger:
      Sinks:
      - Type: Remote
    Name: CanWriter
    ParticipantController:
      SyncType: DistributedTimeQuantum
  - CanControllers:
    - Name: CAN1@CanReader
    Description: Demo Reader
    Logger:
      Sinks:
      - Type: Remote
    Name: CanReader
    ParticipantController:
      SyncType: DistributedTimeQuantum
  - IsSyncMaster: true
    Logger:
      Sinks:
      - Type: Remote
    Name: SystemController
  - Description: Passive SystemMonitor that can be optionally attached. Does not participate
      in synchronization.
    Logger:
      FlushLevel: Info
      LogFromRemotes: true
      Sinks:
      - Type: Stdout
    Name: SystemMonitor
  TimeSync:
    TickPeriodNs: 123456
LaunchConfigurations: []
MiddlewareConfig:
  ActiveMiddleware: VAsio
)raw";

    auto config = Config::FromYamlString(demoYaml);
    EXPECT_TRUE(config.simulationSetup.participants.size() == 4);
    EXPECT_TRUE(config.simulationSetup.timeSync.tickPeriod.count() == 123456);

    EXPECT_TRUE(config.simulationSetup.participants.at(0).name == "CanWriter");
    EXPECT_TRUE(config.simulationSetup.participants.at(1).name == "CanReader");
    EXPECT_TRUE(config.simulationSetup.participants.at(2).name == "SystemController");
    EXPECT_TRUE(config.simulationSetup.participants.at(3).name == "SystemMonitor");

    EXPECT_TRUE(config.simulationSetup.participants.at(0).canControllers.size() == 1);
    EXPECT_TRUE(config.simulationSetup.participants.at(0).canControllers.at(0).name == "CAN1@CanWriter");
    EXPECT_TRUE(config.simulationSetup.participants.at(0).logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(config.simulationSetup.participants.at(1).canControllers.size() == 1);
    EXPECT_TRUE(config.simulationSetup.participants.at(1).canControllers.at(0).name == "CAN1@CanReader");
    EXPECT_TRUE(config.simulationSetup.participants.at(1).logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(config.simulationSetup.participants.at(2).canControllers.size() == 0);
    EXPECT_TRUE(config.simulationSetup.participants.at(2).logger.sinks.at(0).type == Sink::Type::Remote);

    EXPECT_TRUE(config.simulationSetup.participants.at(3).canControllers.size() == 0);
    EXPECT_TRUE(config.simulationSetup.participants.at(3).logger.sinks.at(0).type == Sink::Type::Stdout);
    EXPECT_TRUE(config.simulationSetup.participants.at(3).logger.logFromRemotes);

}

TEST_F(YamlConfigTest, validate_unknown_toplevel)
{
    auto yamlString = R"yaml(
ConfigName: CanDemo
ConfigVersion: 0.0.1
Description: Sample configuration for CAN without NetworkSimulator
#typo in a toplevel statement, additional 's'
SimulationsSetup:
)yaml";

    std::stringstream warnings;
    bool yamlValid = Validate(yamlString, warnings);
    std::cout << "Yaml Validate: warnings: " << warnings.str() << std::endl;
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    EXPECT_TRUE(warnings.str().size() > 0);
}

TEST_F(YamlConfigTest, validate_unnamed_children)
{
    auto yamlString = R"yaml(
SimulationSetup:
  Participants:
    - Name: P1
      CanControllers:
      - Name: CAN1
        UseTraceSinks:
        - Sink1
        #misplaced logger
        Logger:
          Sinks:
          - Type: Remote
      Description:  foo
      #correct place
      Logger:
        Sinks:
        - Type: Remote
)yaml";

    std::stringstream warnings;
    bool yamlValid = Validate(yamlString, warnings);
    EXPECT_FALSE(yamlValid) << "YamlValidator warnings: " << warnings.str();
    std::cout << "YamlValidator warnings: " << warnings.str() <<std::endl;
    EXPECT_TRUE(warnings.str().size() > 0);
}
} // anonymous namespace
