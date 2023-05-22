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

#include "gtest/gtest.h"

#include "ServiceSerdes.hpp"

TEST(MwVAsioSerdes, Mw_Service)
{
    SilKit::Core::MessageBuffer buffer;

    SilKit::Core::Discovery::ParticipantDiscoveryEvent in{};
    in.participantName = "Input";
    for (auto i = 0; i < 10; i++) {
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

