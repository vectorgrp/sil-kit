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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ValidateAndSanitizeConfig.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

TEST(Test_ValidateAndSanitizeConfig, verify_configuration_participant_name_override)
{
    const std::string configuredParticipantName = "Configured Participant Name";
    const std::string parameterParticipantName = "Parameter Participant Name";
    ASSERT_NE(configuredParticipantName, parameterParticipantName);

    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    configuration->participantName = configuredParticipantName;

    auto result = SilKit::Core::ValidateAndSanitizeConfig(
        std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), parameterParticipantName,
        "");

    // make sure the incoming configuration object is not changed
    EXPECT_EQ(configuration->participantName, configuredParticipantName);

    // make sure the sanitized configuration contains the configured participant name
    EXPECT_EQ(result.participantConfiguration.participantName, configuredParticipantName);
}

TEST(Test_ValidateAndSanitizeConfig, verify_empty_configuration_participant_name_override)
{
    const std::string parameterParticipantName = "Parameter Participant Name";
    ASSERT_FALSE(parameterParticipantName.empty());

    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    configuration->participantName = "";
    ASSERT_TRUE(configuration->participantName.empty());

    auto result = SilKit::Core::ValidateAndSanitizeConfig(
        std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), parameterParticipantName,
        "");

    // make sure the incoming configuration object is not changed
    EXPECT_TRUE(configuration->participantName.empty());

    // make sure the sanitized configuration contains the parameter participant name
    EXPECT_EQ(result.participantConfiguration.participantName, parameterParticipantName);
}

TEST(Test_ValidateAndSanitizeConfig,
     verify_empty_parameter_participant_name_throws_with_empty_configured_participant_name)
{
    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    configuration->participantName = "";
    ASSERT_TRUE(configuration->participantName.empty());

    ASSERT_THROW((SilKit::Core::ValidateAndSanitizeConfig(
                     std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), "", "")),
                 SilKit::ConfigurationError);
}

TEST(Test_ValidateAndSanitizeConfig, verify_empty_parameter_participant_name_throws_with_configured_participant_name)
{
    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    configuration->participantName = "Configured Participant Name";
    ASSERT_FALSE(configuration->participantName.empty());

    ASSERT_THROW((SilKit::Core::ValidateAndSanitizeConfig(
                     std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), "", "")),
                 SilKit::ConfigurationError);
}

TEST(Test_ValidateAndSanitizeConfig, verify_configuration_registry_uri_override)
{
    const std::string configuredRegistryUri = "silkit://configured.registry.uri.silkit:1234";
    const std::string parameterRegistryUri = "silkit://parameter.registry.uri.silkit:5678";
    ASSERT_NE(configuredRegistryUri, parameterRegistryUri);

    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    configuration->middleware.registryUri = configuredRegistryUri;

    auto result = SilKit::Core::ValidateAndSanitizeConfig(
        std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), "Participant Name",
        parameterRegistryUri);

    // make sure the incoming configuration object is not changed
    EXPECT_EQ(configuration->middleware.registryUri, configuredRegistryUri);

    // make sure the sanitized configuration contains the configured registry uri
    EXPECT_EQ(result.participantConfiguration.middleware.registryUri, configuredRegistryUri);
}

TEST(Test_ValidateAndSanitizeConfig, verify_empty_configuration_registry_uri_override)
{
    const std::string parameterRegistryUri = "silkit://parameter.registry.uri.silkit:5678";
    ASSERT_FALSE(parameterRegistryUri.empty());

    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    ASSERT_TRUE(configuration->middleware.registryUri.empty());

    auto result = SilKit::Core::ValidateAndSanitizeConfig(
        std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), "Participant Name",
        parameterRegistryUri);

    // make sure the incoming configuration object is not changed
    EXPECT_TRUE(configuration->middleware.registryUri.empty());

    // make sure the sanitized configuration contains the parameter registry uri
    EXPECT_EQ(result.participantConfiguration.middleware.registryUri, parameterRegistryUri);
}

TEST(Test_ValidateAndSanitizeConfig,
     verify_empty_parameter_registry_uri_with_empty_configured_registry_uri_chooses_default)
{
    auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>();
    ASSERT_TRUE(configuration->middleware.registryUri.empty());

    auto result = SilKit::Core::ValidateAndSanitizeConfig(
        std::static_pointer_cast<SilKit::Config::IParticipantConfiguration>(configuration), "Participant Name", "");

    // make sure the sanitized configuration contains a (non-empty) registry uri
    EXPECT_FALSE(result.participantConfiguration.middleware.registryUri.empty());
}

} // anonymous namespace