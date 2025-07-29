// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "ServiceSerdes.hpp"

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
