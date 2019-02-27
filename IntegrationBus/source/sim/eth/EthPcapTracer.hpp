// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <stdint.h>
#include <mutex>
#include <fstream>

#include "EthFrame.hpp"
#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthPcapTracer
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    EthPcapTracer();
    EthPcapTracer(const EthPcapTracer&) = default;
    EthPcapTracer(EthPcapTracer&&) = default;
    ~EthPcapTracer();

    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress);
    void Trace(const EthMessage& message);

private:
    struct {
        uint32_t magic_number  = 0xa1b23c4d;   /* magic number */
        uint16_t version_major = 2;            /* major version number */
        uint16_t version_minor = 4;            /* minor version number */
        int32_t  thiszone      = 0;            /* GMT to local correction */
        uint32_t sigfigs       = 0;            /* accuracy of timestamps */
        uint32_t snaplen       = 65535;        /* max length of captured packets, in octets */
        uint32_t network       = 1;            /* data link type */
    } _pcapGlobalHeader;

    struct {
        uint32_t ts_sec;         /* timestamp seconds */
        uint32_t ts_usec;        /* timestamp microseconds */
        uint32_t incl_len;       /* number of octets of packet saved in file */
        uint32_t orig_len;       /* actual length of packet */
    } _pcapPacketHeader;

    std::ofstream _file;
    std::mutex _lock;
};

}
}
}