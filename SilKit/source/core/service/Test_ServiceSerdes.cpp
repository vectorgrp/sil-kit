// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "core/service/ServiceSerdes.hpp"

TEST(Test_ServiceSerdes, Mw_Service)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::Discovery::ParticipantDiscoveryEvent in{};
    in.participantName = "Input";
    for (auto i = 0; i < 10; i++)
    {
        SilKit::Core::SupplementalData supplementalData;
        supplementalData["hello"] = "world";

        SilKit::Core::ServiceDescriptor descr;
        descr.SetParticipantNameAndComputeId("Participant" + std::to_string(i));
        descr.SetNetworkName("Link" + std::to_string(i));
        descr.SetServiceName("Service" + std::to_string(i));
        descr.SetServiceId(static_cast<SilKit::Core::EndpointId>(i));
        descr.SetServiceType(SilKit::Core::ServiceType::SimulatedController);
        descr.SetSupplementalData(supplementalData);
        descr.SetSupplementalDataItem("Second", "Supplement");
        in.services.push_back(descr);
    }

    SilKit::Core::Discovery::ParticipantDiscoveryEvent out{};

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
    //ensure that sensible values are present
    EXPECT_EQ(out.services.at(9).GetParticipantName(), "Participant9");
    EXPECT_EQ(out.services.at(9).GetSupplementalData().size(), 2u);
    EXPECT_EQ(out.services.at(9).GetSupplementalData().count("hello"), 1u);
    std::string dummy;
    EXPECT_EQ(out.services.at(9).GetSupplementalDataItem("Second", dummy), true);
}

TEST(Test_ServiceDescriptor, generation_is_unset_by_default)
{
    SilKit::Core::ServiceDescriptor descriptor;
    EXPECT_EQ(descriptor.GetGeneration(), std::nullopt);
}

TEST(Test_ServiceDescriptor, get_set_generation)
{
    SilKit::Core::ServiceDescriptor descriptor;
    descriptor.SetSimulationName("Sim");
    descriptor.SetGeneration(42);

    EXPECT_EQ(descriptor.GetGeneration(), 42u);
    EXPECT_EQ(descriptor.GetSimulationName(), "Sim");

    descriptor.SetGeneration(43);
    EXPECT_EQ(descriptor.GetGeneration(), 43u);
    EXPECT_EQ(descriptor.GetSupplementalData().size(), 2u);
}

TEST(Test_ServiceDescriptor, malformed_generation_is_treated_as_unset)
{
    SilKit::Core::ServiceDescriptor descriptor;

    for (const auto* malformed : {"", "abc", "-1", "1x", " 1"})
    {
        descriptor.SetSupplementalDataItem(SilKit::Core::Discovery::generation, malformed);
        EXPECT_EQ(descriptor.GetGeneration(), std::nullopt) << "value: '" << malformed << "'";
    }
}

TEST(Test_ServiceDescriptor, generation_does_not_affect_equality)
{
    SilKit::Core::ServiceDescriptor first;
    first.SetGeneration(23);
    SilKit::Core::ServiceDescriptor second{first};
    second.SetGeneration(42);

    EXPECT_EQ(first, second);
}
