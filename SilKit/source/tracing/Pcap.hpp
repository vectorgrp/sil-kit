// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cstdint>

namespace SilKit {
namespace Tracing {
namespace Pcap {

// The magic number defines the unit of PacketHeader::ts_fraction. SIL Kit writes nanosecond files.
const uint32_t NativeMagic = 0xa1b23c4d;       // nanosecond resolution
const uint32_t MicrosecondMagic = 0xa1b2c3d4;  // microsecond resolution, e.g., written by tcpdump / Wireshark
const size_t GlobalHeaderSize = 24;
const size_t PacketHeaderSize = 16;
const uint16_t MajorVersion = 2;
const uint16_t MinorVersion = 4;

struct GlobalHeader
{
    uint32_t magic_number = NativeMagic;   /* magic number */
    uint16_t version_major = MajorVersion; /* major version number */
    uint16_t version_minor = MinorVersion; /* minor version number */
    int32_t thiszone = 0;                  /* GMT to local correction */
    uint32_t sigfigs = 0;                  /* accuracy of timestamps */
    uint32_t snaplen = 65535;              /* max length of captured packets, in octets */
    uint32_t network = 1;                  /* data link type */
};
static_assert(sizeof(GlobalHeader) == GlobalHeaderSize, "GlobalHeader size must be equal to 24 bytes");

struct PacketHeader
{
    uint32_t ts_sec;      /* timestamp seconds */
    uint32_t ts_fraction; /* timestamp nanoseconds or microseconds, depending on the magic number */
    uint32_t incl_len;    /* number of octets of packet saved in file */
    uint32_t orig_len;    /* actual length of packet */
};
static_assert(sizeof(PacketHeader) == PacketHeaderSize, "PacketHeader size must be equal to 16 bytes");

} // namespace Pcap
} // namespace Tracing
} // namespace SilKit
