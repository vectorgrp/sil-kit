// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthPcapTracer.hpp"

#include <string>
#include <ctime>

namespace ib {
namespace sim {
namespace eth {

EthPcapTracer::EthPcapTracer()
{
}

EthPcapTracer::~EthPcapTracer()
{
    _file.close();
}

void EthPcapTracer::OpenStreams(const std::string& pcapFile, const std::string& pcapPipe)
{
    if (!pcapFile.empty())
    {
        if (_file.is_open())
        {
            _file.close();
        }

        _file.open(pcapFile, std::ios::out | std::ios::binary);
        _file.write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
    }

    if (!pcapPipe.empty())
    {
        _pipe = NamedPipe::Create(pcapPipe);
        _pipe->Write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
    }
}

void EthPcapTracer::Trace(const EthMessage& message)
{
    std::unique_lock<decltype(_lock)> lock;

    pcap::PacketHeader pcapPacketHeader;
    pcapPacketHeader.incl_len = pcapPacketHeader.orig_len = static_cast<uint32_t>(message.ethFrame.GetFrameSize());
    pcapPacketHeader.ts_sec = static_cast<uint32_t>(std::time(nullptr));
    pcapPacketHeader.ts_usec = 0;

    if (_file.is_open())
    {
        _file.write(reinterpret_cast<char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        _file.write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());
    }

    if (_pipe)
    {
        _pipe->Write(reinterpret_cast<char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        _pipe->Write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());
    }
}

}
}
}
