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
    EXPECT_THROW(SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_Logging_Without_File.json"),
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
    auto ref_cfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_FullIncludes_Reference.yaml");

    // cast to ParticipantConfiguration to use right equal operator
    auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
    auto participantConfigRef = *std::dynamic_pointer_cast<ParticipantConfiguration>(ref_cfg);

    ASSERT_TRUE(participantConfig == participantConfigRef);
}

TEST_F(Test_ParticipantConfiguration, participant_config_multiple_acceptor_uris)
{
    EXPECT_THROW(SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_MultipleAcceptorUris.yaml"),
                SilKit::ConfigurationError);
}

TEST_F(Test_ParticipantConfiguration, participant_config_duplicate_controller_names)
{
    EXPECT_THROW(SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_DuplicateControllerNames.yaml"),
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
    )raw";

    auto cfg = SilKit::Config::ParticipantConfigurationFromStringImpl(configString);
    auto ref_cfg = SilKit::Config::ParticipantConfigurationFromFileImpl("ParticipantConfiguration_FromString_Reference.yaml");

    // cast to ParticipantConfiguration to use right equal operator
    auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
    auto participantConfigRef = *std::dynamic_pointer_cast<ParticipantConfiguration>(ref_cfg);

    ASSERT_TRUE(participantConfig == participantConfigRef);

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
}

} // anonymous namespace
