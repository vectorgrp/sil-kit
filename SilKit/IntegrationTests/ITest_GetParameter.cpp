// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/SilKit.hpp"

namespace {

struct ITest_GetParameter : public testing::Test
{

    void checkGetParameterValues(SilKit::IParticipant* participant, std::unordered_map<SilKit::Parameter, std::string> expectedParameters)
    {
        for (auto it : expectedParameters)
        {
            std::string parameterValue = participant->GetParameter(it.first);
            std::string expectedValue = it.second;
            EXPECT_EQ(expectedValue, parameterValue);
        }
    }

    const std::string _registryUriAnyPort = "silkit://127.0.0.1:0";
};

// Check that GetParameter return the values set via api
TEST_F(ITest_GetParameter, get_parameter_set_by_api)
{
    const std::string participantNameByApi = "P1";

    auto emptyParticipantConfig = SilKit::Config::ParticipantConfigurationFromString("");

    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(emptyParticipantConfig);
    auto registryUriByApi = registry->StartListening(_registryUriAnyPort);

    auto participant = SilKit::CreateParticipant(emptyParticipantConfig, participantNameByApi, registryUriByApi);

    checkGetParameterValues(participant.get(), {{SilKit::Parameter::ParticipantName, participantNameByApi},
                                                {SilKit::Parameter::RegistryUri, registryUriByApi}});
}

// Config values take precedence over api values
// Check that GetParameter actually return the config values if both are set
TEST_F(ITest_GetParameter, get_parameter_set_by_config)
{
    const std::string participantNameByApi = "P2";
    const std::string registryUriByApi = "silkit://127.0.0.42:0";

    const std::string participantNameByConfig = "P1";

    auto emptyParticipantConfig = SilKit::Config::ParticipantConfigurationFromString("");

    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(emptyParticipantConfig);
    auto registryUriByConfig = registry->StartListening(_registryUriAnyPort);

    std::ostringstream ss;
    ss << R"({ "ParticipantName": ")" << participantNameByConfig << R"(", "Middleware": { "RegistryUri": ")"
       << registryUriByConfig << R"(" }})";
    auto participantConfig = SilKit::Config::ParticipantConfigurationFromString(ss.str());

    auto participant = SilKit::CreateParticipant(participantConfig, participantNameByApi, registryUriByApi);

    checkGetParameterValues(participant.get(), {{SilKit::Parameter::ParticipantName, participantNameByConfig},
                                                {SilKit::Parameter::RegistryUri, registryUriByConfig}});

}

} //end namespace
