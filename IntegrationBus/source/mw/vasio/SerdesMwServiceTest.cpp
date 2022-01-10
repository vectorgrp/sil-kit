// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesMwService.hpp"

#include "gtest/gtest.h"

TEST(MwVAsioSerdes, Mw_Service)
{
    ib::mw::MessageBuffer buffer;

    ib::mw::service::ServiceAnnouncement in{};
    in.participantName = "Input";
    for (auto i = 0; i < 10; i++) {
        ib::mw::ServiceDescriptor descr;
        descr.participantName = "Participant" + std::to_string(i);
        descr.serviceName = "Service" + std::to_string(i);
        descr.linkName = "Link" + std::to_string(i);
        descr.isLinkSimulated = true;
        descr.legacyEpa.participant = i;
        descr.legacyEpa.endpoint = i;
        descr.supplementalData["hello"] = "world";
        descr.supplementalData["Second"] = "Supplement";
        in.services.push_back(descr);
    }

    ib::mw::service::ServiceAnnouncement out{};
    
    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
    //ensure that sensible values are present
    EXPECT_EQ(out.services.at(9).participantName, "Participant9");
    EXPECT_EQ(out.services.at(9).supplementalData.size(), 2);
}

