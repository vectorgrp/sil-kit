// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "EthPcapTracer.hpp"

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

void EthPcapTracer::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    if (_file.is_open())
    {
        _file.close();
    }

    std::string path = std::to_string(endpointAddress.participant) + ".pcap";
    _file.open(path, std::ios::out | std::ios::binary);
    _file.write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
}

void EthPcapTracer::Trace(const EthMessage& message)
{
    _lock.lock();

    _pcapPacketHeader.incl_len = _pcapPacketHeader.orig_len = message.ethFrame.GetFrameSize();
    _pcapPacketHeader.ts_sec  = static_cast<uint32_t>(message.timestamp.count() / 1'000'000'000LL);
    // magic number in _pcapGlobalHeader implicits nanoseconds here
    _pcapPacketHeader.ts_usec = static_cast<uint32_t>(message.timestamp.count() % 1'000'000'000LL);
    _file.write(reinterpret_cast<char*>(&_pcapPacketHeader), sizeof(_pcapPacketHeader));

    _file.write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());

    _lock.unlock();
}

}
}
}
