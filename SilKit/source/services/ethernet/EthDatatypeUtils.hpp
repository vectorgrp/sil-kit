// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "WireEthernetMessages.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

using EthernetMac = std::array<uint8_t, 6>;

inline bool operator==(const EthernetFrame& lhs, const EthernetFrame& rhs)
{
    return Util::ItemsAreEqual(lhs.raw, rhs.raw);
}

inline bool operator==(const EthernetFrameEvent& lhs, const EthernetFrameEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.frame == rhs.frame;
}

inline bool operator==(const EthernetFrameTransmitEvent& lhs, const EthernetFrameTransmitEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.status == rhs.status;
}

inline bool operator==(const EthernetSetMode& lhs, const EthernetSetMode& rhs)
{
    return lhs.mode == rhs.mode;
}

inline bool operator==(const EthernetStateChangeEvent& lhs, const EthernetStateChangeEvent& rhs)
{
    return lhs.state == rhs.state && lhs.timestamp == rhs.timestamp;
}

inline bool operator==(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs)
{
    return lhs.bitrate == rhs.bitrate && lhs.timestamp == rhs.timestamp;
}

//! Used by NeMatcher in EthControllerProxyTest
inline bool operator!=(const EthernetBitrateChangeEvent& lhs, const EthernetBitrateChangeEvent& rhs)
{
    return lhs.bitrate != rhs.bitrate || lhs.timestamp != rhs.timestamp;
}

//! \brief The EtherType field in a frame can indicate the protocol, payload size, or the start of a VLAN tag
using EthernetEtherType = uint16_t;

//! \brief The Frame Check Sequence field in a frame denotes a 32bit CRC computed as a function of all other fields
using EthernetFrameCheckSequence = std::array<uint8_t, 4>;

//! \brief The Tag Control Identifier field in a frame consists of 3bits Priority code point (PCP), 1bit Drop eligible indicator (DEI), and 12bits VLAN identifier (VID)
using EthernetVlanTagControlIdentifier = uint16_t;

const EthernetEtherType EthernetEtherTypeVlanTag = 0x8100;

const size_t EthernetFrameVlanTagSize = sizeof(EthernetEtherTypeVlanTag) + sizeof(EthernetVlanTagControlIdentifier);

//! \brief Size of an Ethernet level 2 frame header in bytes, excluding optional VLAN tags
const size_t EthernetFrameHeaderSize = 2 * sizeof(EthernetMac) + sizeof(EthernetEtherType);

//! \brief Build an Ethernet level 2 frame without VLAN tag
inline auto CreateEthernetFrame(const SilKit::Services::Ethernet::EthernetMac& destinationMac,
                                const SilKit::Services::Ethernet::EthernetMac& sourceMac,
                                const SilKit::Services::Ethernet::EthernetEtherType& etherType,
                                const std::string& payload) -> SilKit::Services::Ethernet::WireEthernetFrame
{
    std::vector<uint8_t> raw;

    raw.reserve(EthernetFrameHeaderSize + payload.size());
    std::copy(destinationMac.begin(), destinationMac.end(), std::back_inserter(raw));
    std::copy(sourceMac.begin(), sourceMac.end(), std::back_inserter(raw));
    auto etherTypeBytes = reinterpret_cast<const uint8_t*>(&etherType);
    raw.push_back(etherTypeBytes[1]); // We assume our platform to be little-endian
    raw.push_back(etherTypeBytes[0]);
    std::copy(payload.begin(), payload.end(), std::back_inserter(raw));

    return WireEthernetFrame{raw};
}

//! \brief Build an Ethernet level 2 frame with VLAN tag
inline auto CreateEthernetFrameWithVlanTag(const SilKit::Services::Ethernet::EthernetMac& destinationMac,
                                           const SilKit::Services::Ethernet::EthernetMac& sourceMac,
                                           const SilKit::Services::Ethernet::EthernetEtherType& etherType,
                                           const std::string& payload, const EthernetVlanTagControlIdentifier& tci)
    -> SilKit::Services::Ethernet::WireEthernetFrame
{
    std::vector<uint8_t> raw;

    raw.reserve(EthernetFrameHeaderSize + payload.size());
    std::copy(destinationMac.begin(), destinationMac.end(), std::back_inserter(raw));
    std::copy(sourceMac.begin(), sourceMac.end(), std::back_inserter(raw));
    auto etherTypeVlanTagBytes = reinterpret_cast<const uint8_t*>(&EthernetEtherTypeVlanTag);
    raw.push_back(etherTypeVlanTagBytes[1]); // We assume our platform to be little-endian
    raw.push_back(etherTypeVlanTagBytes[0]);
    auto tciBytes = reinterpret_cast<const uint8_t*>(&tci);
    raw.push_back(tciBytes[1]); // We assume our platform to be little-endian
    raw.push_back(tciBytes[0]);
    auto etherTypeBytes = reinterpret_cast<const uint8_t*>(&etherType);
    raw.push_back(etherTypeBytes[1]); // We assume our platform to be little-endian
    raw.push_back(etherTypeBytes[0]);
    std::copy(payload.begin(), payload.end(), std::back_inserter(raw));

    return WireEthernetFrame{raw};
}

inline void SetDestinationMac(std::vector<uint8_t>& raw, const EthernetMac& source)
{
    const size_t MinFrameSize = 60;
    if (raw.empty())
    {
        raw.resize(MinFrameSize);
    }

    ::memcpy(raw.data(), source.data(), sizeof(EthernetMac));
}

inline void SetSourceMac(std::vector<uint8_t>& raw, const EthernetMac& source)
{
    const size_t MinFrameSize = 60;
    const size_t SourceMacStart = sizeof(EthernetMac);
    if (raw.empty())
    {
        raw.resize(MinFrameSize);
    }

    std::copy(source.begin(), source.end(), raw.begin() + SourceMacStart);
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
