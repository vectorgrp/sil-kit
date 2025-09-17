// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/can/all.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Can;

// This is the common behavior used in CanReaderDemo and CanWriterDemo
namespace CanDemoCommon {

void FrameTransmitHandler(const CanFrameTransmitEvent& canFrameAck, ILogger* logger)
{
    std::stringstream ss;
    ss << "Receive CAN frame transmit acknowledge: canId=" << canFrameAck.canId << ", status='" << canFrameAck.status
       << "'";
    logger->Info(ss.str());
}

void FrameHandler(const CanFrameEvent& canFrameEvent, ILogger* logger, bool printHex)
{
    std::string frameTypeHint = "";
    if ((canFrameEvent.frame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)) != 0)
    {
        frameTypeHint = "FD ";
    }
    if ((canFrameEvent.frame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Xlf)) != 0)
    {
        frameTypeHint = "XL ";
    }
    std::stringstream ss;
    ss << "Receive CAN " << frameTypeHint << "frame: canId=" << canFrameEvent.frame.canId << ", data=";
    if (printHex)
    {
        ss << "[" << Util::AsHexString(canFrameEvent.frame.dataField).WithSeparator(" ") << "]";
    }
    else
    {
        ss << "'" << std::string(canFrameEvent.frame.dataField.begin(), canFrameEvent.frame.dataField.end()) << "'";
    }
    logger->Info(ss.str());
}

} // namespace CanDemoCommon
