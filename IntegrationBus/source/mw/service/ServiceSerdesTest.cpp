// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gtest/gtest.h"

#include "ServiceSerdes.hpp"

TEST(MwVAsioSerdes, Mw_Service)
{
    ib::mw::MessageBuffer buffer;

    ib::mw::service::ParticipantDiscoveryEvent in{};
    in.participantName = "Input";
    for (auto i = 0; i < 10; i++) {
        ib::mw::SupplementalData supplementalData;
        supplementalData["hello"] = "world";

        ib::mw::ServiceDescriptor descr;
        descr.SetParticipantName("Participant" + std::to_string(i));
        descr.SetNetworkName("Link" + std::to_string(i));
        descr.SetServiceName("Service" + std::to_string(i));
        descr.SetServiceId(static_cast<ib::mw::EndpointId>(i));
        descr.SetServiceType(ib::mw::ServiceType::SimulatedController);
        descr.SetSupplementalData(supplementalData);
        descr.SetSupplementalDataItem("Second", "Supplement");
        in.services.push_back(descr);
    }

    ib::mw::service::ParticipantDiscoveryEvent out{};
    
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

