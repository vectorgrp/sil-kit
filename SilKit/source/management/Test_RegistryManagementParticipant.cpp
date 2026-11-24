// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "management/RegistryManagementParticipant.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace SilKit::Experimental::Management;
using testing::ElementsAre;
using testing::IsEmpty;

auto Names(const Simulations& simulations) -> std::vector<std::string>
{
    std::vector<std::string> names;
    for (const auto& simulation : simulations.simulations)
    {
        names.push_back(simulation.name);
    }
    return names;
}

auto RoundTrip(const Simulations& value) -> Simulations
{
    return Deserialize<Simulations>(Serialize(value));
}

class Test_RegistryManagementParticipant : public testing::Test
{
protected:
    VSilKit::RegistryManagementParticipant participant{nullptr, false};
};

TEST_F(Test_RegistryManagementParticipant, simulations_follow_participant_connections)
{
    participant.OnParticipantConnected("b", "B1");
    participant.OnParticipantConnected("a", "A2");
    participant.OnParticipantConnected("a", "A1");

    auto simulations = participant.GetSimulations();
    EXPECT_THAT(Names(simulations), ElementsAre("a", "b"));
    EXPECT_THAT(simulations.simulations[0].participantNames, ElementsAre("A1", "A2"));
    EXPECT_THAT(simulations.simulations[1].participantNames, ElementsAre("B1"));
    const auto sequence = simulations.sequence;

    participant.OnParticipantDisconnected("b", "B1");
    participant.OnParticipantDisconnected("a", "A1");

    simulations = participant.GetSimulations();
    EXPECT_THAT(Names(simulations), ElementsAre("a"));
    EXPECT_THAT(simulations.simulations[0].participantNames, ElementsAre("A2"));
    EXPECT_GT(simulations.sequence, sequence);
}

TEST_F(Test_RegistryManagementParticipant, well_known_simulations_are_not_reported)
{
    participant.OnParticipantConnected(SimulationName(), RegistryParticipantName());
    participant.OnParticipantConnected(".well-known/other", "P");

    EXPECT_THAT(participant.GetSimulations().simulations, IsEmpty());
}

TEST_F(Test_RegistryManagementParticipant, required_participants_are_kept_per_simulation)
{
    participant.OnParticipantConnected("a", "Controller");
    participant.OnParticipantConnected("b", "B1");

    const std::vector<std::string> required{"Controller", "A1"};
    participant.OnRequiredParticipantsUpdate("a", "Controller", required);
    participant.OnRequiredParticipantsUpdate("unknown", "Controller", required);

    const auto simulations = participant.GetSimulations();
    EXPECT_THAT(Names(simulations), ElementsAre("a", "b"));
    EXPECT_THAT(simulations.simulations[0].requiredParticipantNames, ElementsAre("Controller", "A1"));
    EXPECT_THAT(simulations.simulations[1].requiredParticipantNames, IsEmpty());
}

TEST_F(Test_RegistryManagementParticipant, participant_configuration_status)
{
    participant.OnParticipantConfiguration("a", "Shared", R"({"ParticipantName":"Shared"})");
    participant.OnParticipantConnected("a", "Shared");
    participant.OnParticipantConnected("a", "NotShared");

    auto reply = participant.GetParticipantConfiguration({"a", "Shared"});
    EXPECT_EQ(reply.status, ParticipantConfigurationStatus::Available);
    EXPECT_EQ(reply.configurationJson, R"({"ParticipantName":"Shared"})");

    reply = participant.GetParticipantConfiguration({"a", "NotShared"});
    EXPECT_EQ(reply.status, ParticipantConfigurationStatus::NotShared);
    EXPECT_THAT(reply.configurationJson, IsEmpty());

    EXPECT_EQ(participant.GetParticipantConfiguration({"a", "Unknown"}).status,
              ParticipantConfigurationStatus::UnknownParticipant);
    EXPECT_EQ(participant.GetParticipantConfiguration({"b", "Shared"}).status,
              ParticipantConfigurationStatus::UnknownParticipant);

    participant.OnParticipantDisconnected("a", "Shared");
    EXPECT_EQ(participant.GetParticipantConfiguration({"a", "Shared"}).status,
              ParticipantConfigurationStatus::UnknownParticipant);
}

TEST_F(Test_RegistryManagementParticipant, registry_info_reports_api_version_and_features)
{
    const auto info = participant.GetRegistryInfo();
    EXPECT_EQ(info.apiVersionMajor, ApiVersionMajor());
    EXPECT_EQ(info.apiVersionMinor, ApiVersionMinor());
    EXPECT_FALSE(info.silKitVersion.empty());
    EXPECT_THAT(info.features,
                ElementsAre(Registry::FeatureSimulations(), Registry::FeatureParticipantConfiguration()));
}

TEST(Test_ManagementDatatypes, payloads_round_trip)
{
    RegistryInfo info{1, 2, "5.0.8", {"x", "y"}};
    const auto infoCopy = Deserialize<RegistryInfo>(Serialize(info));
    EXPECT_EQ(infoCopy.apiVersionMajor, 1u);
    EXPECT_EQ(infoCopy.apiVersionMinor, 2u);
    EXPECT_EQ(infoCopy.silKitVersion, "5.0.8");
    EXPECT_THAT(infoCopy.features, ElementsAre("x", "y"));

    Simulations simulations;
    simulations.sequence = 42;
    simulations.simulations.push_back({"a", {"A1", "A2"}, {"A1"}});
    simulations.simulations.push_back({"b/c", {}, {}});
    const auto simulationsCopy = RoundTrip(simulations);
    EXPECT_EQ(simulationsCopy.sequence, 42u);
    EXPECT_THAT(Names(simulationsCopy), ElementsAre("a", "b/c"));
    EXPECT_THAT(simulationsCopy.simulations[0].participantNames, ElementsAre("A1", "A2"));
    EXPECT_THAT(simulationsCopy.simulations[0].requiredParticipantNames, ElementsAre("A1"));
    EXPECT_THAT(simulationsCopy.simulations[1].participantNames, IsEmpty());

    const auto request =
        Deserialize<ParticipantConfigurationRequest>(Serialize(ParticipantConfigurationRequest{"a", "A1"}));
    EXPECT_EQ(request.simulationName, "a");
    EXPECT_EQ(request.participantName, "A1");

    const auto reply = Deserialize<ParticipantConfigurationReply>(
        Serialize(ParticipantConfigurationReply{ParticipantConfigurationStatus::NotShared, "{}"}));
    EXPECT_EQ(reply.status, ParticipantConfigurationStatus::NotShared);
    EXPECT_EQ(reply.configurationJson, "{}");
}

TEST(Test_ManagementDatatypes, management_uri_replaces_simulation_path)
{
    EXPECT_EQ(MakeManagementUri("silkit://localhost:8500"), "silkit://localhost:8500/.well-known/sil-kit/management");
    EXPECT_EQ(MakeManagementUri("silkit://localhost:8500/"), "silkit://localhost:8500/.well-known/sil-kit/management");
    EXPECT_EQ(MakeManagementUri("silkit://[::1]:8500/some/sim"), "silkit://[::1]:8500/.well-known/sil-kit/management");
}

} // namespace
