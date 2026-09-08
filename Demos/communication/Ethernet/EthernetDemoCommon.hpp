// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Ethernet;

// This is the common behavior used in EthernetReaderDemo and EthernetWriterDemo
namespace EthernetDemoCommon {

using EtherType = uint16_t;
using EthernetMac = std::array<uint8_t, 6>;

// Deliberately malformed Ethernet frames, used by EthernetWriterDemo --invalid to exercise a receiver's frame
// validation.
//
// Each case violates exactly one check of CANoe's GetEthPacketInvalidReason
// (projects_source/CANoe/Source/RTEVENT/SilKit/SilKitEth.cpp) and makes it report write message 83-0303 with the
// quoted reason in its Details.
//
// One reason of that function is deliberately absent here: "the Ethernet header could not be parsed (header length
// below 14 bytes)" cannot be reached by a sender. The size check (>= 60 bytes) runs first, and for a packet of that
// size the header walker always advances past 14 bytes before reporting the header end - it starts at offset 12 and
// consumes either a 2 byte EtherType or a tag of >= 4 bytes. Only a receiver-side unit test reaches that reason.
namespace InvalidFrames {

static constexpr EthernetMac kWriterMac{0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1};
static constexpr EthernetMac kBroadcastMac{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//! Smallest packet CANoe accepts (NEthernet::kMinPacketSize).
static constexpr size_t kMinPacketSize = 60;
//! Largest packet CANoe accepts (NEthernet::kMaxPacketSize_Standard = 1500 + 112).
static constexpr size_t kMaxPacketSize = 1612;

//! dst MAC + src MAC + EtherType, padded with 0xAB up to totalSize.
inline auto MakeFrame(size_t totalSize, EtherType etherType = 0x0000) -> std::vector<uint8_t>
{
    std::vector<uint8_t> raw;
    std::copy(kBroadcastMac.begin(), kBroadcastMac.end(), std::back_inserter(raw));
    std::copy(kWriterMac.begin(), kWriterMac.end(), std::back_inserter(raw));
    raw.push_back(static_cast<uint8_t>(etherType >> 8));
    raw.push_back(static_cast<uint8_t>(etherType & 0xFF));
    raw.resize(std::max(raw.size(), totalSize), 0xAB);
    return raw;
}

struct Case
{
    const char* name;
    const char* expectedReason; //!< reason text the receiver is expected to report
    std::function<std::vector<uint8_t>()> Build;
};

inline auto All() -> const std::vector<Case>&
{
    static const std::vector<Case> cases{
        // raw.data == nullptr is checked before the size, so an empty frame gives this reason, not "too small".
        {"no-data", "SIL Kit did not provide any packet data", []() { return std::vector<uint8_t>{}; }},
        {"too-small", "size is below the minimum Ethernet packet size of 60 bytes",
         []() { return MakeFrame(kMinPacketSize - 20); }},
        {"too-large", "size exceeds the maximum supported Ethernet packet size of 1612 bytes",
         []() { return MakeFrame(kMaxPacketSize + 1); }},
        // Four stacked 802.1QinQ S-tags (0x88A8 + 2 byte TCI each), then a payload EtherType that ends the header
        // walk. CANoe allows at most 3 S-tags.
        {"vlan-stags", "the packet carries more than 3 VLAN S-tags",
         []() {
        std::vector<uint8_t> raw;
        std::copy(kBroadcastMac.begin(), kBroadcastMac.end(), std::back_inserter(raw));
        std::copy(kWriterMac.begin(), kWriterMac.end(), std::back_inserter(raw));
        for (int tag = 0; tag < 4; ++tag)
        {
            raw.push_back(0x88); // IEEE 802.1QinQ S-tag EtherType
            raw.push_back(0xA8);
            raw.push_back(0x00); // TCI: PCP / DEI / VID
            raw.push_back(static_cast<uint8_t>(tag + 1));
        }
        raw.push_back(0x08); // payload EtherType: IPv4
        raw.push_back(0x00);
        raw.resize(std::max(raw.size(), kMinPacketSize), 0xAB); // stay above the minimum size
        return raw;
    }},
    };
    return cases;
}

inline auto Find(const std::string& name) -> const Case*
{
    for (const auto& c : All())
    {
        if (c.name == name)
        {
            return &c;
        }
    }
    return nullptr;
}

inline auto NameList() -> std::string
{
    std::string list;
    for (const auto& c : All())
    {
        list += (list.empty() ? "" : ", ");
        list += c.name;
    }
    return list;
}

} // namespace InvalidFrames

inline void FrameTransmitHandler(const EthernetFrameTransmitEvent& frameTransmitEvent, ILogger* logger)
{
    std::stringstream ss;
    if (frameTransmitEvent.status == EthernetTransmitStatus::Transmitted)
    {
        ss << "Received ACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext;
    }
    else
    {
        ss << "Received NACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext;
        switch (frameTransmitEvent.status)
        {
        case EthernetTransmitStatus::Transmitted:
            break;
        case EthernetTransmitStatus::InvalidFrameFormat:
            ss << ": InvalidFrameFormat";
            break;
        case EthernetTransmitStatus::ControllerInactive:
            ss << ": ControllerInactive";
            break;
        case EthernetTransmitStatus::LinkDown:
            ss << ": LinkDown";
            break;
        case EthernetTransmitStatus::Dropped:
            ss << ": Dropped";
            break;
        }
    }
    logger->Info(ss.str());
}

inline auto PrintPayload(const std::vector<uint8_t>& payload, bool printHex)
{
    std::stringstream ss;
    if (printHex)
    {
        ss << "[" << Util::AsHexString(payload).WithSeparator(" ") << "]";
    }
    else
    {
        ss << "'" << std::string(payload.begin(), payload.end()) << "'";
    }
    return ss.str();
}

inline void FrameHandler(const EthernetFrameEvent& ethernetFrameEvent, ILogger* logger, bool printHex)
{
    const size_t FrameHeaderSize = 2 * sizeof(EthernetMac) + sizeof(EtherType);
    std::vector<uint8_t> payloadWithoutHeader;
    payloadWithoutHeader.insert(payloadWithoutHeader.end(), ethernetFrameEvent.frame.raw.begin() + FrameHeaderSize,
                                ethernetFrameEvent.frame.raw.end());

    std::stringstream ss;
    ss << "Receive Ethernet frame, data=" << PrintPayload(payloadWithoutHeader, printHex);
    logger->Info(ss.str());
}

} // namespace EthernetDemoCommon
