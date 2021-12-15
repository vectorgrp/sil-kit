// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesMwService.hpp"

#include "gtest/gtest.h"

TEST(MwVAsioSerdes, Mw_Service)
{
    ib::mw::MessageBuffer buffer;

    ib::mw::service::ServiceAnnouncement in{};
    in.participantName = "Input";
    for (auto i = 0; i < 10; i++) {
        ib::mw::service::ServiceDescription descr;
        descr.serviceId.participantName = "Participant" + std::to_string(i);
        descr.serviceId.serviceName = "Service" + std::to_string(i);
        descr.serviceId.linkName = "Link" + std::to_string(i);
        descr.serviceId.isLinkSimulated = true;
        descr.serviceId.legacyEpa.participant = i;
        descr.serviceId.legacyEpa.endpoint = i;
        descr.supplementalData["hello"] = "world";
        descr.supplementalData["Second"] = "Supplement";
        in.services.push_back(descr);
    }

    ib::mw::service::ServiceAnnouncement out{};
    
    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
    EXPECT_EQ(out.services.at(9).serviceId.participantName, "Participant9");
}

