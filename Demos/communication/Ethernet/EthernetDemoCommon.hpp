// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Ethernet;

// This is the common behavior used in EthernetReaderDemo and EthernetWriterDemo
namespace EthernetDemoCommon {

using EtherType = uint16_t;
using EthernetMac = std::array<uint8_t, 6>;

void FrameTransmitHandler(const EthernetFrameTransmitEvent& frameTransmitEvent, ILogger* logger)
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

auto PrintPayload(const std::vector<uint8_t>& payload, bool printHex)
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

void FrameHandler(const EthernetFrameEvent& ethernetFrameEvent, ILogger* logger, bool printHex)
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
