// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"
#include "PcapSink.hpp"
#include "detail/NamedPipe.hpp"

#include <string>
#include <ctime>

namespace ib {
namespace tracing {


namespace pcap
{
struct GlobalHeader {
    uint32_t magic_number = 0xa1b23c4d;  /* magic number */
    uint16_t version_major = 2;          /* major version number */
    uint16_t version_minor = 4;          /* minor version number */
    int32_t  thiszone = 0;               /* GMT to local correction */
    uint32_t sigfigs = 0;                /* accuracy of timestamps */
    uint32_t snaplen = 65535;            /* max length of captured packets, in octets */
    uint32_t network = 1;                /* data link type */
};
static_assert(sizeof(GlobalHeader) == 24, "GlobalHeader size must be equal to 24 bytes");

struct PacketHeader {
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
};
static_assert(sizeof(PacketHeader) == 16, "PacketHeader size must be equal to 16 bytes");
} // namespace pcap

namespace {
pcap::GlobalHeader g_pcapGlobalHeader{};
} //anonymous namespace

PcapSink::PcapSink(mw::logging::ILogger* logger, std::string name)
    : _name{std::move(name)}
    , _logger{logger}
{

}

void PcapSink::Open(tracing::SinkType outputType, const std::string& outputPath)
{
    if (outputPath.empty())
    {
        throw std::runtime_error("PcapSink::Open: outputPath must not be empty!");
    }

    switch (outputType)
    {
    case ib::tracing::SinkType::PcapFile:
        if (_file.is_open())
        {
            _file.close();
        }
        _file.open(outputPath, std::ios::out | std::ios::binary);
        _file.write(reinterpret_cast<char*>(&g_pcapGlobalHeader),
                sizeof(g_pcapGlobalHeader));
        break;
    case ib::tracing::SinkType::PcapNamedPipe:
        _logger->Info("Sink {}: Waiting for a reader to connect to PCAP pipe {} ... ", _name, outputPath);
        _pipe = NamedPipe::Create(outputPath);
        _pipe->Write(reinterpret_cast<char*>(&g_pcapGlobalHeader),
                sizeof(g_pcapGlobalHeader));
        _logger->Debug("Sink {}: PCAP pipe: {} is connected successfully", _name, outputPath);
        break;
    default:
        throw std::runtime_error("PcapSink::Open: specified SinkType not implemented");
    }
}
void PcapSink::Close()
{
    if (_file)
    {
        _file.flush();
        _file.close();
    }

    if (_pipe)
    {
        _pipe.reset();
    }
}

void PcapSink::Trace(tracing::Direction /*unused*/,
        const mw::EndpointAddress& /* unusued endpoind address */,
        std::chrono::nanoseconds timestamp,
        const sim::eth::EthFrame& message)
{
    std::unique_lock<decltype(_lock)> lock;

    const auto tosec = 1000'000ull;
    const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(timestamp);

    pcap::PacketHeader pcapPacketHeader;
    pcapPacketHeader.orig_len = static_cast<uint32_t>(message.GetFrameSize());
    pcapPacketHeader.incl_len = pcapPacketHeader.orig_len;
    pcapPacketHeader.ts_sec = usec.count() / tosec;
    pcapPacketHeader.ts_usec = usec.count() % tosec;

    bool ok = true;
    if (_file.is_open())
    {
        _file.write(reinterpret_cast<char*>(&pcapPacketHeader),
                sizeof(pcapPacketHeader));
        _file.write(reinterpret_cast<const char*>(&message.RawFrame().at(0)),
                message.GetFrameSize());
        ok = _file.good();
    }

    if (_pipe)
    {
        ok &=_pipe->Write(reinterpret_cast<char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        ok &=_pipe->Write(reinterpret_cast<const char*>(&message.RawFrame().at(0)), message.GetFrameSize());
    }
}


} // tracing
} // ib
