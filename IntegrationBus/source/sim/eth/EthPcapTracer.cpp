// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthPcapTracer.hpp"

#include <string>
#include <ctime>
#include <iostream>

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

    auto path = std::to_string(endpointAddress.participant) + ".pcap";
    _file.open(path, std::ios::out | std::ios::binary);
    _file.write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));

    if (endpointAddress.participant == 2)
    {
        _pipe = NamedPipe::Create("wireshark");
        _pipe->Write(reinterpret_cast<char*>(&_pcapGlobalHeader), sizeof(_pcapGlobalHeader));
    }
}

void EthPcapTracer::Trace(const EthMessage& message)
{
    _lock.lock();

    _pcapPacketHeader.incl_len = _pcapPacketHeader.orig_len = static_cast<uint32_t>(message.ethFrame.GetFrameSize());
    _pcapPacketHeader.ts_sec = static_cast<uint32_t>(std::time(nullptr));
    _pcapPacketHeader.ts_usec = 0;
    
    _file.write(reinterpret_cast<char*>(&_pcapPacketHeader), sizeof(_pcapPacketHeader));
    _file.write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());

    if (_pipe)
    {
        auto success = _pipe->Write(reinterpret_cast<const char*>(&_pcapPacketHeader), sizeof(_pcapPacketHeader));
        success |= _pipe->Write(reinterpret_cast<const char*>(&message.ethFrame.RawFrame().at(0)), message.ethFrame.GetFrameSize());

        if (!success)
        {
            std::cout << "NamedPipe::Write was not successfull!" << std::endl;
        }
    }

    _lock.unlock();
}

}
}
}
