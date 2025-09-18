// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cstdint>

namespace SilKit {
namespace Tracing {
namespace Pcap {

const uint32_t NativeMagic = 0xa1b23c4d;
const size_t GlobalHeaderSize = 24;
const size_t PacketHeaderSize = 16;
const uint16_t MajorVersion = 2;
const uint16_t MinorVersion = 4;

struct GlobalHeader
{
// magic number
    uint32_t magic_number = NativeMagic;  
// major version number
    uint16_t version_major = MajorVersion;
// minor version number
    uint16_t version_minor = MinorVersion;
// GMT to local correction
    int32_t thiszone = 0;                 
// accuracy of timestamps
    uint32_t sigfigs = 0;                 
// max length of captured packets, in octets
    uint32_t snaplen = 65535;             
// data link type
    uint32_t network = 1;                 
};
static_assert(sizeof(GlobalHeader) == GlobalHeaderSize, "GlobalHeader size must be equal to 24 bytes");

struct PacketHeader
{
// timestamp seconds
    uint32_t ts_sec;  
// timestamp microseconds
    uint32_t ts_usec; 
// number of octets of packet saved in file
    uint32_t incl_len;
// actual length of packet
    uint32_t orig_len;
};
static_assert(sizeof(PacketHeader) == PacketHeaderSize, "PacketHeader size must be equal to 16 bytes");

} // namespace Pcap
} // namespace Tracing
} // namespace SilKit
