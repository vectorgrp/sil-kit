/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
    uint32_t magic_number = NativeMagic; /* magic number */
    uint16_t version_major = MajorVersion; /* major version number */
    uint16_t version_minor = MinorVersion; /* minor version number */
    int32_t thiszone = 0; /* GMT to local correction */
    uint32_t sigfigs = 0; /* accuracy of timestamps */
    uint32_t snaplen = 65535; /* max length of captured packets, in octets */
    uint32_t network = 1; /* data link type */
};
static_assert(sizeof(GlobalHeader) == GlobalHeaderSize, "GlobalHeader size must be equal to 24 bytes");

struct PacketHeader
{
    uint32_t ts_sec; /* timestamp seconds */
    uint32_t ts_usec; /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */
};
static_assert(sizeof(PacketHeader) == PacketHeaderSize, "PacketHeader size must be equal to 16 bytes");

} // namespace Pcap
} // namespace Tracing
} // namespace SilKit
