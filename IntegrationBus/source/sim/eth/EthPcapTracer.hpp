// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <mutex>
#include <fstream>

#include "EthFrame.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "NamedPipe.hpp"

namespace ib {
namespace sim {
namespace eth {
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
}


class EthPcapTracer
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    EthPcapTracer();
    EthPcapTracer(const EthPcapTracer&) = default;
    EthPcapTracer(EthPcapTracer&&) = default;
    ~EthPcapTracer();

    // ----------------------------------------
    // Public methods
    void OpenStreams(const std::string& pcapFile, const std::string& pcapPipe);
    void Trace(const EthMessage& message);

private:
    // ----------------------------------------
    // Private members
    std::ofstream _file;
    NamedPipe::Ptr _pipe;

    std::mutex _lock;

    pcap::GlobalHeader _pcapGlobalHeader;
};

}
}
}