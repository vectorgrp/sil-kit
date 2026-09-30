// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Automates the QA test cases 2.23.1 "PCAP Tracing" and 2.23.2 "PCAP Pipe Tracing": Ethernet frames of a controller
// with a PCAP trace sink end up in a valid PCAP stream. The stream is parsed here without SIL Kit internals, the same
// way an external tool like Wireshark reads it.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "silkit/services/all.hpp"

#include "SimTestHarness.hpp"
#include "EthernetHelpers.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#if defined(_WIN32)
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <windows.h>
#else
#    include <fcntl.h>
#    include <unistd.h>
#endif

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Ethernet;
using SilKit::IntegrationTests::CreateEthernetFrameFromString;
using SilKit::IntegrationTests::EthernetEtherType;
using SilKit::IntegrationTests::EthernetFrameHeaderSize;
using SilKit::IntegrationTests::EthernetMac;

constexpr uint32_t pcapMagicMicroseconds = 0xa1b2c3d4;
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
void RunEthernetSimulation(const std::string& writerConfig)
{
    SilKit::Tests::SimTestHarnessArgs args;
    args.syncParticipantNames = {"EthWriter", "EthReader"};
    args.deferParticipantCreation = true;
    SilKit::Tests::SimTestHarness testHarness{args};

    auto* writer = testHarness.GetParticipant("EthWriter", writerConfig);
    auto* writerController = writer->Participant()->CreateEthernetController("ETH1", "LINK1");
    writer->GetOrCreateLifecycleService()->SetCommunicationReadyHandler(
        [writerController] { writerController->Activate(); });

    size_t numSent = 0;
    writer->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [writerController, &numSent](std::chrono::nanoseconds, std::chrono::nanoseconds) {
        if (numSent < numberOfFrames)
        {
            const EthernetMac destination{0x12, 0x23, 0x45, 0x67, 0x89, 0x9a};
            const EthernetMac source{0x9a, 0x89, 0x67, 0x45, 0x23, 0x12};
            const EthernetEtherType etherType{0x0800};
            writerController->SendFrame(
                EthernetFrame{CreateEthernetFrameFromString(destination, source, etherType, MessageForFrame(numSent))});
            numSent++;
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

    ASSERT_TRUE(testHarness.Run(10s));
    EXPECT_EQ(numReceived, numberOfFrames);
    // Destroying the participants closes the trace sinks.
    testHarness.ResetParticipants();
}

void ExpectFramesInPcap(const PcapStream& pcap)
{
    EXPECT_THAT(pcap.magic, testing::AnyOf(pcapMagicMicroseconds, pcapMagicNanoseconds));
    EXPECT_EQ(pcap.versionMajor, 2u);
    EXPECT_EQ(pcap.versionMinor, 4u);
    EXPECT_EQ(pcap.linkType, linkTypeEthernet);

    ASSERT_EQ(pcap.packets.size(), numberOfFrames) << "Each sent frame must be traced exactly once";
    for (size_t i = 0; i < pcap.packets.size(); ++i)
    {
        const auto& data = pcap.packets[i].data;
        ASSERT_GE(data.size(), EthernetFrameHeaderSize);
        const std::string payload(data.begin() + EthernetFrameHeaderSize, data.end());
        EXPECT_EQ(payload, MessageForFrame(i));
        if (i > 0)
        {
            const auto& previous = pcap.packets[i - 1];
            EXPECT_TRUE(std::tie(previous.tsSec, previous.tsFraction)
                        < std::tie(pcap.packets[i].tsSec, pcap.packets[i].tsFraction))
                << "Frames are sent at increasing simulation times";
        }
    }
}

TEST(ITest_PcapTracing, pcap_file_sink_contains_all_sent_frames)
{
    const auto path = std::filesystem::current_path()
                      / ("ITest_PcapTracing_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())
                         + ".pcap");

    RunEthernetSimulation(MakeWriterConfig("PcapFile", path.generic_string()));

    ASSERT_TRUE(std::filesystem::exists(path)) << "The PCAP file was not created";
    ExpectFramesInPcap(ParsePcap(ReadBinaryFile(path)));

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

// Reads the whole PCAP stream from the named pipe the sink creates. The sink writes the first bytes when the first
// frame is traced and blocks until a reader is connected.
class PipeReader
{
public:
    explicit PipeReader(std::string pipeName)
        : _thread{[this, pipeName] { Read(pipeName); }}
    {
    }

    ~PipeReader()
    {
        _abort = true;
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    auto Join() -> std::vector<uint8_t>
    {
        _thread.join();
        return _bytes;
    }

private:
    void Read(const std::string& pipeName)
    {
        const auto deadline = std::chrono::steady_clock::now() + 10s;
#if defined(_WIN32)
        const auto path = "\\\\.\\pipe\\" + pipeName;
        HANDLE handle = INVALID_HANDLE_VALUE;
        while (handle == INVALID_HANDLE_VALUE && !_abort && std::chrono::steady_clock::now() < deadline)
        {
            handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (handle == INVALID_HANDLE_VALUE)
            {
                std::this_thread::sleep_for(10ms);
            }
        }
        if (handle == INVALID_HANDLE_VALUE)
        {
            return;
        }
        // Reading ends with ERROR_BROKEN_PIPE when the sink closes the pipe.
        char buffer[65536];
        DWORD numRead = 0;
        while (ReadFile(handle, buffer, sizeof(buffer), &numRead, nullptr) || GetLastError() == ERROR_MORE_DATA)
        {
            _bytes.insert(_bytes.end(), buffer, buffer + numRead);
            numRead = 0;
        }
        CloseHandle(handle);
#else
        while (!std::filesystem::exists(pipeName) && !_abort && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(10ms);
        }
        const int fd = ::open(pipeName.c_str(), O_RDONLY);
        if (fd < 0)
        {
            return;
        }
        char buffer[65536];
        ssize_t numRead = 0;
        while ((numRead = ::read(fd, buffer, sizeof(buffer))) > 0)
        {
            _bytes.insert(_bytes.end(), buffer, buffer + numRead);
        }
        ::close(fd);
#endif
    }

    std::atomic<bool> _abort{false};
    std::vector<uint8_t> _bytes;
    std::thread _thread;
};

TEST(ITest_PcapTracing, pcap_pipe_sink_streams_all_sent_frames)
{
    const auto uniqueName = "ITest_PcapPipe_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
#if defined(_WIN32)
    const auto pipeName = uniqueName;
#else
    const auto pipeName = (std::filesystem::temp_directory_path() / uniqueName).string();
#endif

    PipeReader pipeReader{pipeName};
    RunEthernetSimulation(MakeWriterConfig("PcapPipe", pipeName));

    ExpectFramesInPcap(ParsePcap(pipeReader.Join()));
}

} // namespace
