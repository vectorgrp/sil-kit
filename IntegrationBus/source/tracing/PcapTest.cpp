// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "PcapReader.hpp"

#include <cstring>

#include "silkit/services/eth/EthernetDatatypes.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Pcap.hpp"
#include "MockParticipant.hpp"


namespace {
using namespace SilKit::tracing;
using namespace SilKit::Services::Ethernet;
using namespace SilKit::Core::Tests;

std::vector<char> MakePcapTestData(EthernetFrame& frame, size_t numMessages)
{
    std::vector<char> data;
    std::vector<uint8_t> payload(1500, '\0');

    Pcap::GlobalHeader ghdr{};
    Pcap::PacketHeader phdr{};

    frame.SetDestinationMac(EthernetMac{1,2,3,4,5,6});
    frame.SetSourceMac(EthernetMac{7,8,9,0xa,0xb,0xc});
    frame.SetEtherType(0x0800);
    frame.SetPayload(payload);

    data.resize(
        Pcap::GlobalHeaderSize
        + (Pcap::PacketHeaderSize + frame.RawFrame().size()) * numMessages
    );
    // encode
    auto* raw = data.data();
    memcpy(raw, &ghdr, Pcap::GlobalHeaderSize);
    raw += Pcap::GlobalHeaderSize;
    for (auto i = 0u; i < numMessages; i++)
    {
        phdr.ts_sec = i;
        phdr.ts_usec = i;
        phdr.incl_len = frame.RawFrame().size();
        phdr.orig_len = phdr.incl_len;
        memcpy(raw, &phdr, Pcap::PacketHeaderSize);
        raw += Pcap::PacketHeaderSize;

        memcpy(raw, frame.RawFrame().data(), frame.RawFrame().size());
        raw += frame.RawFrame().size();
    }
    return data;
}

TEST(ReplayTest, read_from_pcap)
{
    EthernetFrame frame{};
    DummyLogger log;
    std::stringstream ss;

    auto raw = MakePcapTestData(frame, 10);
    ss.write(raw.data(), raw.size());

    PcapReader reader{&ss, &log};

    auto numMessages = 0u;
    while (true)
    {
        auto msg = reader.Read();
        if (!msg)
        {
            break;
        }
        numMessages++;

        //validate contents of messages
        auto ethMsg = dynamic_cast<EthernetFrame&>(*msg);
        EXPECT_EQ(ethMsg.GetEtherType(), frame.GetEtherType());
        EXPECT_EQ(ethMsg.GetSourceMac(), frame.GetSourceMac());
        EXPECT_EQ(ethMsg.GetDestinationMac(), frame.GetDestinationMac());
        EXPECT_EQ(ethMsg.RawFrame(), frame.RawFrame());


        if (!reader.Seek(1))
        {
            break;
        }
    }
    EXPECT_EQ(numMessages, 10);
}

} //end anonymous namespace
