// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/ethernet/all.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Ethernet;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    out << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << "ms";
    return out;
}

// This is the common behavior used in EthernetReaderDemo and EthernetWriterDemo
namespace EthernetDemoCommon {

using EtherType = uint16_t;
using EthernetMac = std::array<uint8_t, 6>;

void FrameTransmitHandler(const EthernetFrameTransmitEvent& frameTransmitEvent, ILogger* logger)
{
    std::stringstream ss;
    if (frameTransmitEvent.status == EthernetTransmitStatus::Transmitted)
    {
        ss << ">> ACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext << std::endl;
    }
    else
    {
        ss << ">> NACK for Ethernet frame with userContext=" << frameTransmitEvent.userContext;
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

        ss << std::endl;
        logger->Info(ss.str());
    }
}

std::string GetPayloadStringFromFrame(const EthernetFrame& frame)
{
    const size_t FrameHeaderSize = 2 * sizeof(EthernetMac) + sizeof(EtherType);

    std::vector<uint8_t> payload;
    payload.insert(payload.end(), frame.raw.begin() + FrameHeaderSize, frame.raw.end());
    std::string payloadString(payload.begin(), payload.end());
    return payloadString;
}

void FrameHandler(const EthernetFrameEvent& ethernetFrameEvent, ILogger* logger)
{
    auto payload = GetPayloadStringFromFrame(ethernetFrameEvent.frame);
    std::stringstream ss;
    ss << ">> Ethernet frame: \"" << payload << "\"" << std::endl;
    logger->Info(ss.str());
}

} // namespace EthernetDemoBehavior
