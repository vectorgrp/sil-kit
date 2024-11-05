// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/can/all.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Can;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

// This is the common behavior used in CanReaderDemo and CanWriterDemo
namespace CanDemoCommon {

void FrameTransmitHandler(const CanFrameTransmitEvent& canFrameAck, ILogger* logger)
{
    // Log
    std::stringstream buffer;
    buffer << "Ack CAN frame, canId=" << canFrameAck.canId << ", status='" << canFrameAck.status << "'";
    logger->Info(buffer.str());
}

void FrameHandler(const CanFrameEvent& canFrameEvent, ILogger* logger)
{
    // Indicate frame type in log message
    std::string frameTypeHint = "";
    if ((canFrameEvent.frame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)) != 0)
    {
        frameTypeHint = "FD ";
    }

    // Log
    std::string payloadStr(canFrameEvent.frame.dataField.begin(), canFrameEvent.frame.dataField.end());
    std::stringstream buffer;
    buffer << "Receive CAN " << frameTypeHint << "frame, canId=" << canFrameEvent.frame.canId << ", data='"
           << payloadStr << "'";
    logger->Info(buffer.str());
}

} // namespace CanDemoBehavior
