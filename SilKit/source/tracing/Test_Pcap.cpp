// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "PcapReader.hpp"

#include <cstring>

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Pcap.hpp"
#include "MockParticipant.hpp"
#include "EthDatatypeUtils.hpp"

namespace {

using namespace SilKit::Tracing;
using namespace SilKit::Services::Ethernet;
using namespace SilKit::Core::Tests;

std::vector<uint8_t> MakePcapTestData(WireEthernetFrame& wireFrame, size_t numMessages)
{
    std::vector<uint8_t> data;
    std::string payloadData{"Testing Trace message Pcap, padding to a minimal size of > 64 bytes"
                            "... and so on 0123456789ABCDEF"};

    Pcap::GlobalHeader ghdr{};
    Pcap::PacketHeader phdr{};

    EthernetMac destinationMac{1, 2, 3, 4, 5, 6};
    EthernetMac sourceMac{7, 8, 9, 0xa, 0xb, 0xc};
    EthernetEtherType etherType{0x0800};

    wireFrame = CreateEthernetFrame(destinationMac, sourceMac, etherType, payloadData);
    auto frame = wireFrame.raw.AsSpan();

    data.resize(Pcap::GlobalHeaderSize + (Pcap::PacketHeaderSize + frame.size()) * numMessages);
    // encode
    auto* raw = data.data();
    memcpy(raw, &ghdr, Pcap::GlobalHeaderSize);
    raw += Pcap::GlobalHeaderSize;
    for (auto i = 0u; i < numMessages; i++)
    {
        phdr.ts_sec = i;
        phdr.ts_usec = i;
        phdr.incl_len = static_cast<uint32_t>(frame.size());
        phdr.orig_len = phdr.incl_len;
        memcpy(raw, &phdr, Pcap::PacketHeaderSize);
        raw += Pcap::PacketHeaderSize;

        memcpy(raw, frame.data(), frame.size());
        raw += frame.size();
    }
    return data;
}

TEST(Test_Pcap, read_from_pcap)
{
    MockLogger log;
    std::stringstream ss;

    WireEthernetFrame testInput;
    auto raw = MakePcapTestData(testInput, 10);
    ss.write(reinterpret_cast<char*>(raw.data()), raw.size());

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
        auto ethMsg = dynamic_cast<WireEthernetFrame&>(*msg);
        ASSERT_TRUE(ItemsAreEqual(ethMsg.raw.AsSpan(), testInput.raw.AsSpan()));

        if (!reader.Seek(1))
        {
            break;
        }
    }
    EXPECT_EQ((int)numMessages, 10);
}

} // namespace
