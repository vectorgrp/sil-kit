// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// PCAP tracing to a file and to a pipe: Ethernet frames of a controller with a PCAP trace sink end up in a valid PCAP
// stream. The stream is parsed here without SIL Kit internals, the same way an external tool like Wireshark reads it.

#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "silkit/services/all.hpp"

#include "SimTestHarness.hpp"
#include "EthernetHelpers.hpp"
#include "IntegrationTestUtils.hpp"

#include "gtest/gtest.h"


namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Ethernet;
using SilKit::IntegrationTests::CreateEthernetFrameFromString;
using SilKit::IntegrationTests::EthernetEtherType;
using SilKit::IntegrationTests::EthernetFrameHeaderSize;
using SilKit::IntegrationTests::EthernetMac;

constexpr uint32_t pcapMagicNanoseconds = 0xa1b23c4d;
constexpr uint32_t linkTypeEthernet = 1;
constexpr size_t numberOfFrames = 5;

struct PcapPacket
{
    uint32_t tsSec;
    uint32_t tsFraction;
    std::vector<uint8_t> data;
};

struct PcapStream
{
    uint32_t magic{0};
    uint16_t versionMajor{0};
    uint16_t versionMinor{0};
    uint32_t linkType{0};
    std::vector<PcapPacket> packets;
};

// Parses a little-endian PCAP stream (https://www.ietf.org/archive/id/draft-ietf-opsawg-pcap-04.html).
auto ParsePcap(const std::vector<uint8_t>& bytes) -> PcapStream
{
    PcapStream result;
    size_t offset = 0;
    auto read32 = [&bytes, &offset]() {
        uint32_t value;
        std::memcpy(&value, bytes.data() + offset, sizeof(value));
        offset += sizeof(value);
        return value;
    };
    auto read16 = [&bytes, &offset]() {
        uint16_t value;
        std::memcpy(&value, bytes.data() + offset, sizeof(value));
        offset += sizeof(value);
        return value;
    };

    if (bytes.size() < 24)
    {
        ADD_FAILURE() << "PCAP stream is shorter than its global header: " << bytes.size() << " bytes";
        return result;
    }
    result.magic = read32();
    result.versionMajor = read16();
    result.versionMinor = read16();
    offset += 4 + 4 + 4; // thiszone, sigfigs, snaplen
    result.linkType = read32();

    while (offset + 16 <= bytes.size())
    {
        PcapPacket packet;
        packet.tsSec = read32();
        packet.tsFraction = read32();
        const auto includedLength = read32();
        offset += 4; // original length
        if (offset + includedLength > bytes.size())
        {
            ADD_FAILURE() << "Truncated PCAP packet record";
            break;
        }
        packet.data.assign(bytes.begin() + offset, bytes.begin() + offset + includedLength);
        offset += includedLength;
        result.packets.push_back(std::move(packet));
    }
    EXPECT_EQ(offset, bytes.size()) << "Trailing bytes after the last PCAP packet record";
    return result;
}

auto ReadBinaryFile(const std::filesystem::path& path) -> std::vector<uint8_t>
{
    std::ifstream stream{path, std::ios::binary};
    return std::vector<uint8_t>{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
}

auto MessageForFrame(size_t index) -> std::string
{
    // Pad to the minimum Ethernet frame length of 60 bytes.
    std::string message = "PCAP test frame " + std::to_string(index);
    message.resize(60 - EthernetFrameHeaderSize, ' ');
    return message;
}

auto MakeWriterConfig(const std::string& sinkType, const std::string& outputPath) -> std::string
{
    return R"(
EthernetControllers:
- Name: ETH1
  Network: LINK1
  UseTraceSinks:
  - PcapSink
Tracing:
  TraceSinks:
  - Name: PcapSink
    Type: )" + sinkType + R"(
    OutputPath: )" + outputPath + "\n";
}

// EthWriter sends one frame per millisecond, EthReader stops the simulation after receiving all of them.
auto RunEthernetSimulation(const std::string& writerConfig) -> std::vector<std::chrono::nanoseconds>
{
    SilKit::Tests::SimTestHarnessArgs args;
    args.syncParticipantNames = {"EthWriter", "EthReader"};
    args.deferParticipantCreation = true;
    SilKit::Tests::SimTestHarness testHarness{args};

    auto* writer = testHarness.GetParticipant("EthWriter", writerConfig);
    auto* writerController = writer->Participant()->CreateEthernetController("ETH1", "LINK1");
    writer->GetOrCreateLifecycleService()->SetCommunicationReadyHandler(
        [writerController] { writerController->Activate(); });

    std::vector<std::chrono::nanoseconds> sendTimes;
    writer->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [writerController, &sendTimes](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        if (sendTimes.size() < numberOfFrames)
        {
            const EthernetMac destination{0x12, 0x23, 0x45, 0x67, 0x89, 0x9a};
            const EthernetMac source{0x9a, 0x89, 0x67, 0x45, 0x23, 0x12};
            const EthernetEtherType etherType{0x0800};
            writerController->SendFrame(
                EthernetFrame{CreateEthernetFrameFromString(destination, source, etherType,
                                                            MessageForFrame(sendTimes.size()))});
            sendTimes.push_back(now);
        }
    }, 1ms);

    auto* reader = testHarness.GetParticipant("EthReader", "");
    auto* readerController = reader->Participant()->CreateEthernetController("ETH1", "LINK1");
    reader->GetOrCreateLifecycleService()->SetCommunicationReadyHandler(
        [readerController] { readerController->Activate(); });
    reader->GetOrCreateTimeSyncService()->SetSimulationStepHandler([](auto, auto) {}, 1ms);

    size_t numReceived = 0;
    readerController->AddFrameHandler([reader, &numReceived](IEthernetController*, const EthernetFrameEvent&) {
        if (++numReceived == numberOfFrames)
        {
            reader->Stop();
        }
    });

    EXPECT_TRUE(testHarness.Run(10s));
    EXPECT_EQ(numReceived, numberOfFrames);
    // Destroying the participants closes the trace sinks.
    testHarness.ResetParticipants();
    return sendTimes;
}

void ExpectFramesInPcap(const PcapStream& pcap, const std::vector<std::chrono::nanoseconds>& sendTimes)
{
    // The magic number announces the unit of the timestamp fraction. External tools like Wireshark rely on it.
    ASSERT_EQ(pcap.magic, pcapMagicNanoseconds);
    EXPECT_EQ(pcap.versionMajor, 2u);
    EXPECT_EQ(pcap.versionMinor, 4u);
    EXPECT_EQ(pcap.linkType, linkTypeEthernet);

    ASSERT_EQ(sendTimes.size(), numberOfFrames);
    ASSERT_EQ(pcap.packets.size(), numberOfFrames) << "Each sent frame must be traced exactly once";
    for (size_t i = 0; i < pcap.packets.size(); ++i)
    {
        const auto& packet = pcap.packets[i];
        ASSERT_GE(packet.data.size(), EthernetFrameHeaderSize);
        const std::string payload(packet.data.begin() + EthernetFrameHeaderSize, packet.data.end());
        EXPECT_EQ(payload, MessageForFrame(i));

        const auto timestamp = std::chrono::seconds{packet.tsSec} + std::chrono::nanoseconds{packet.tsFraction};
        EXPECT_EQ(timestamp, sendTimes[i]) << "The packet timestamp must be the simulation time the frame was sent at";
    }
}

TEST(ITest_PcapTracing, pcap_file_sink_contains_all_sent_frames)
{
    const auto path = std::filesystem::current_path()
                      / ("ITest_PcapTracing_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())
                         + ".pcap");

    const auto sendTimes = RunEthernetSimulation(MakeWriterConfig("PcapFile", path.generic_string()));

    ASSERT_TRUE(std::filesystem::exists(path)) << "The PCAP file was not created";
    ExpectFramesInPcap(ParsePcap(ReadBinaryFile(path)), sendTimes);

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

// The sink writes the first bytes when the first frame is traced and blocks until a reader is connected.
TEST(ITest_PcapTracing, pcap_pipe_sink_streams_all_sent_frames)
{
    const auto uniqueName = "ITest_PcapPipe_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
#if defined(_WIN32)
    const auto pipeName = uniqueName;
#else
    const auto pipeName = (std::filesystem::temp_directory_path() / uniqueName).string();
#endif

    IntegrationTestUtils::PipeReader pipeReader{pipeName};
    const auto sendTimes = RunEthernetSimulation(MakeWriterConfig("PcapPipe", pipeName));

    ExpectFramesInPcap(ParsePcap(pipeReader.Join()), sendTimes);
}

} // namespace
