// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthPcapTracer.hpp"

#include <string>
#include <ctime>

namespace ib {
namespace sim {
namespace eth {

EthPcapTracer::EthPcapTracer(const std::string& pcapFile, const std::string& pcapPipe)
{
    if (!pcapFile.empty())
        _fileName = pcapFile + ".pcap";

    if (!pcapPipe.empty())
        _pipeName = pcapPipe;
}

EthPcapTracer::~EthPcapTracer()
{
    _file.close();
}

void EthPcapTracer::OpenStreams()
{
    if (!_fileName.empty())
    {
        if (_file.is_open())
        {
            _file.close();
        }

        _file.open(_fileName, std::ios::out | std::ios::binary);
        _file.write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
    }

    if (!_pipeName.empty())
    {
        _pipe = NamedPipe::Create(_pipeName);
        _pipe->Write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
    }
}

void EthPcapTracer::Trace(const EthMessage& message)
{
    if (_fileName.empty() && _pipeName.empty())
    {
        return;
    }

    _lock.lock();

    _pcapPacketHeader.incl_len = _pcapPacketHeader.orig_len = static_cast<uint32_t>(message.ethFrame.GetFrameSize());
    _pcapPacketHeader.ts_sec = static_cast<uint32_t>(std::time(nullptr));
    _pcapPacketHeader.ts_usec = 0;

    if (_file.is_open())
    {
        _file.write(reinterpret_cast<char*>(&_pcapPacketHeader), sizeof(_pcapPacketHeader));
        _file.write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());
    }

    if (_pipe)
    {
        _pipe->Write(reinterpret_cast<char*>(&_pcapPacketHeader), sizeof(_pcapPacketHeader));
        _pipe->Write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());
    }

    _lock.unlock();
}

}
}
}
