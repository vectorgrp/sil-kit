// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>
#include <memory>

#include "NullConnectionParticipant.hpp"
#include "ParticipantConfiguration.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "silkit/services/all.hpp"
#include "functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace SilKit::Config;

class Test_ParticipantConfiguration : public testing::Test
{
protected:
    Test_ParticipantConfiguration() {}

    void CreateParticipantFromConfiguration(std::shared_ptr<IParticipantConfiguration> cfg)
    {
        auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
        (void)SilKit::Core::CreateNullConnectionParticipantImpl(cfg, participantConfig.participantName);
    }
};

TEST_F(Test_ParticipantConfiguration, throw_if_logging_is_configured_without_filename)
{
    EXPECT_THROW(
        SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Logging_Without_File.json"),
        SilKit::ConfigurationError);
}

TEST_F(Test_ParticipantConfiguration, minimal_configuration_file)
{
    auto cfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Minimal.json");
    CreateParticipantFromConfiguration(cfg);
}

TEST_F(Test_ParticipantConfiguration, full_configuration_file)
{
    auto cfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Full.json");
    CreateParticipantFromConfiguration(cfg);
}

TEST_F(Test_ParticipantConfiguration, full_configuration_file_with_includes)
{
    auto cfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_FullIncludes.yaml");
    auto ref_cfg =
        SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_FullIncludes_Reference.yaml");

    // cast to ParticipantConfiguration to use right equal operator
    auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
    auto participantConfigRef = *std::dynamic_pointer_cast<ParticipantConfiguration>(ref_cfg);

    ASSERT_EQ(participantConfig, participantConfigRef);
}

TEST_F(Test_ParticipantConfiguration, participant_config_multiple_acceptor_uris)
{
    EXPECT_THROW(
        SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_MultipleAcceptorUris.yaml"),
        SilKit::ConfigurationError);
}

TEST_F(Test_ParticipantConfiguration, participant_config_duplicate_controller_names)
{
    EXPECT_THROW(
        SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_DuplicateControllerNames.yaml"),
        SilKit::ConfigurationError);
}

TEST_F(Test_ParticipantConfiguration, participant_config_from_string)
{
    const auto configString = R"raw(
---
Description: Example include configuration for CAN Controllers
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
HealthCheck:
  SoftResponseTimeout: 1234
  HardResponseTimeout: 4567
Extensions:
  SearchPathHints:
  - Foobar
Tracing:
  TraceSinks:
  - Name: sink1
    Type: Mdf4File
    OutputPath: someFile.mf4
 
    )raw";

    auto cfg = SilKit::Config::ParticipantConfigurationFromStringImpl(configString);
    auto ref_cfg =
        SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_FromString_Reference.yaml");

    // cast to ParticipantConfiguration to use right equal operator
    auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
    auto participantConfigRef = *std::dynamic_pointer_cast<ParticipantConfiguration>(ref_cfg);

    ASSERT_EQ(participantConfig, participantConfigRef);
}
TEST_F(Test_ParticipantConfiguration, participant_config_from_string_includes)
{
    const auto configString = R"raw(
---
Description: Example include configuration for CAN Controllers
Includes:
  SearchPathHints:
    - ConfigSnippets/CAN
  Files:
      - CanInclude5.yaml
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
    )raw";

    const auto configStringRef = R"raw(
---
Description: Example include configuration for CAN Controllers
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
- Name: CAN64
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel64
      ChannelPath: path/to/myTestChannel64
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup64
      GroupSource: MyTestGroup
    )raw";

    auto cfg = SilKit::Config::ParticipantConfigurationFromStringImpl(configString);
    auto cfgRef = SilKit::Config::ParticipantConfigurationFromStringImpl(configStringRef);

    // cast to ParticipantConfiguration to use right equal operator
    auto config = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
    auto configRef = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfgRef);
    ASSERT_TRUE(config == configRef);
}

TEST_F(Test_ParticipantConfiguration, participant_config_logging_sinks)
{
    const auto configString = R"raw(
---
Description: Example include configuration for CAN Controllers
Includes:
  Files:
      - LoggingIncludes.yaml
Logging:
  Sinks:
    - Type: Stdout
      Level: Info
CanControllers:
- Name: CAN1
  Replay:
    UseTraceSource: Source1
    Direction: Both
    MdfChannel:
      ChannelName: MyTestChannel1
      ChannelPath: path/to/myTestChannel1
      ChannelSource: MyTestChannel
      GroupName: MyTestGroup
      GroupPath: path/to/myTestGroup1
      GroupSource: MyTestGroup
  UseTraceSinks:
  - Sink1
- Name: MyCAN2
  Network: CAN2
    )raw";

    EXPECT_THROW(SilKit::Config::ParticipantConfigurationFromStringImpl(configString), SilKit::ConfigurationError);
}

/*
    Test whether json and yaml configurations are parsed equivalently
*/
TEST_F(Test_ParticipantConfiguration, full_configuration_file_json_yaml_equal)
{
    auto jsonCfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Full.json");
    auto yamlCfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Full.yaml");

    // cast to ParticipantConfiguration to use right equal operator
    auto participantConfigJson = *std::dynamic_pointer_cast<ParticipantConfiguration>(jsonCfg);
    auto participantConfigYaml = *std::dynamic_pointer_cast<ParticipantConfiguration>(yamlCfg);

    ASSERT_TRUE(participantConfigJson == participantConfigYaml);
    EXPECT_EQ(participantConfigYaml.healthCheck.hardResponseTimeout.value(), 5000ms);
    EXPECT_EQ(participantConfigYaml.healthCheck.softResponseTimeout.value(), 500ms);
    EXPECT_EQ(participantConfigYaml.extensions.searchPathHints.at(1), "path/to/extensions2");
}

TEST_F(Test_ParticipantConfiguration, remote_metric_sink_collect_from_remote_fails)
{
    constexpr auto configurationString = R"(
Experimental:
    Metrics:
      CollectFromRemote: true
      Sinks:
        - Type: Remote
)";

    ASSERT_THROW(SilKit::Config::ParticipantConfigurationFromStringImpl(configurationString),
                 SilKit::ConfigurationError);
}

void CheckEmpty(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
{
    auto c = *std::dynamic_pointer_cast<ParticipantConfiguration>(config);
    EXPECT_EQ(c.description, "");
    ASSERT_EQ(c.participantName, "");
    ASSERT_EQ(c.logging.sinks.size(), 0u);
    ASSERT_EQ(c.schemaVersion, "");
}

TEST_F(Test_ParticipantConfiguration, empty_json)
{
    auto&& config = SilKit::Config::ParticipantConfigurationFromStringImpl("{}");
    CheckEmpty(config);
}

TEST_F(Test_ParticipantConfiguration, empty_yaml)
{
    auto&& config = SilKit::Config::ParticipantConfigurationFromStringImpl("");
    CheckEmpty(config);
}

} // anonymous namespace
