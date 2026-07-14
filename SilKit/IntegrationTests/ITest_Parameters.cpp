// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

namespace {

class ITest_Parameters : public testing::Test
{
protected:
    const std::string _registryUriAnyPort{"silkit://127.0.0.1:0"};
};

TEST_F(ITest_Parameters, participant_name_and_registry_uri_set_by_api)
{
    const std::string participantNameByApi{"P1"};

    auto emptyParticipantConfig = SilKit::Config::ParticipantConfigurationFromString("");
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(emptyParticipantConfig);
    auto registryUriByApi = registry->StartListening(_registryUriAnyPort);

    auto participant = SilKit::CreateParticipant(emptyParticipantConfig, participantNameByApi, registryUriByApi);

    EXPECT_EQ(participant->GetParticipantName(), participantNameByApi);
    EXPECT_EQ(participant->GetRegistryUri(), registryUriByApi);
}

TEST_F(ITest_Parameters, participant_name_and_registry_uri_set_by_configuration)
{
    const std::string participantNameByApi{"P2"};
    const std::string registryUriByApi{"silkit://127.0.0.42:0"};
    const std::string participantNameByConfig{"P1"};

    auto emptyParticipantConfig = SilKit::Config::ParticipantConfigurationFromString("");
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(emptyParticipantConfig);
    auto registryUriByConfig = registry->StartListening(_registryUriAnyPort);

    std::ostringstream ss;
    ss << R"({ "ParticipantName": ")" << participantNameByConfig
       << R"(", "Middleware": { "RegistryUri": ")" << registryUriByConfig << R"(" }})";

    auto participantConfig = SilKit::Config::ParticipantConfigurationFromString(ss.str());
    auto participant = SilKit::CreateParticipant(participantConfig, participantNameByApi, registryUriByApi);

    EXPECT_EQ(participant->GetParticipantName(), participantNameByConfig);
    EXPECT_EQ(participant->GetRegistryUri(), registryUriByConfig);
}

} // namespace
