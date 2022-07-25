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

#include "PcapReader.hpp"

#include <cstring>

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

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
