// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>
#include <memory>

#include "NullConnectionParticipant.hpp"
#include "ParticipantConfiguration.hpp"
#include "silkit/services/all.hpp"
#include "silkit/util/functional.hpp"

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

class ParticipantConfigurationExamplesITest : public testing::Test
{
protected:
    ParticipantConfigurationExamplesITest() {}

    void CreateParticipantFromConfiguration(std::shared_ptr<IParticipantConfiguration> cfg)
    {
        auto participantConfig = *std::dynamic_pointer_cast<ParticipantConfiguration>(cfg);
        (void)SilKit::Core::CreateNullConnectionParticipantImpl(cfg, participantConfig.participantName);
    }
};

TEST_F(ParticipantConfigurationExamplesITest, throw_if_logging_is_configured_without_filename)
{
    EXPECT_THROW(SilKit::Config::ParticipantConfigurationFromFile("ParticipantConfiguration_Logging_Without_File.json"),
                 SilKit::ConfigurationError);
}

TEST_F(ParticipantConfigurationExamplesITest, minimal_configuration_file)
{
    auto cfg = SilKit::Config::ParticipantConfigurationFromFile("ParticipantConfiguration_Minimal.json");
    CreateParticipantFromConfiguration(cfg);
}

TEST_F(ParticipantConfigurationExamplesITest, full_configuration_file)
{
    auto cfg = SilKit::Config::ParticipantConfigurationFromFile("ParticipantConfiguration_Full.json");
    CreateParticipantFromConfiguration(cfg);
}

} // anonymous namespace
