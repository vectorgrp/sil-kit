// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "silkit/services/can/all.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Can;

// This is the common behavior used in CanReaderDemo and CanWriterDemo
namespace CanDemoCommon {

// Deliberately malformed CAN frames, used by CanWriterDemo --invalid to exercise a receiver's frame validation.
//
// Each case violates exactly one check of CANoe's GetCanFrameInvalidReason
// (projects_source/CANoe/Source/RTEVENT/SilKit/SilKitCAN.cpp) and makes it report write message 83-0203 with the
// quoted reason in its Details. The checks are evaluated in a fixed order - dlc, then canId, then the flag
// combinations, then the payload size - so every case keeps the fields of the earlier checks legal.
//
// Note: SilKit_CanFrame::dlc is documented as "determined by a network simulator if available". Send these frames
// in a simulation *without* a network simulator, otherwise the frame is normalised before the receiver sees it.
namespace InvalidFrames {

struct Case
{
    const char* name;
    const char* expectedReason; //!< reason text the receiver is expected to report
    std::function<void(CanFrame& frame, std::vector<uint8_t>& payload)> Build;
};

inline auto Mask(CanFrameFlag flag) -> CanFrameFlagMask
{
    return static_cast<CanFrameFlagMask>(flag);
}

inline auto All() -> const std::vector<Case>&
{
    static const std::vector<Case> cases{
        {"xl-dlc", "dlc exceeds the maximum of 2047 for CAN XL frames",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Fdf) | Mask(CanFrameFlag::Xlf);
        frame.dlc = 2048;
    }},
        {"dlc", "dlc exceeds the maximum of 15 for CAN and CAN FD frames",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Fdf);
        frame.dlc = 16;
    }},
        {"ext-id", "id exceeds the maximum of 0x1FFFFFFF for extended identifiers",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Ide);
        frame.canId = 0x20000000;
    }},
        {"std-id", "id exceeds the maximum of 0x7FF for standard identifiers (ide is not set)",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = 0;
        frame.canId = 0x800;
    }},
        {"fd-rtr", "rtr is set, but CAN FD does not have RTR frames",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Fdf) | Mask(CanFrameFlag::Rtr);
    }},
        {"brs", "brs is set, but it is valid for CAN FD frames only (fdf is not set)",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Brs);
    }},
        {"esi", "esi is set, but it is valid for CAN FD frames only (fdf is not set)",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Esi);
    }},
        {"xlf", "xlf is set, but CAN XL requires fdf to be set as well",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Xlf);
    }},
        // Sec is only checked against Xlf, so Fdf has to be set to get past the "xlf requires fdf" check first.
        {"sec", "sec is set, but it is valid for CAN XL frames only (xlf is not set)",
         [](CanFrame& frame, std::vector<uint8_t>&) {
        frame.flags = Mask(CanFrameFlag::Fdf) | Mask(CanFrameFlag::Sec);
    }},
        {"datasize", "datasize does not match the payload length derived from dlc and the protocol flags",
         [](CanFrame& frame, std::vector<uint8_t>& payload) {
        frame.flags = 0;
        frame.dlc = 8; // a classic frame with dlc 8 must carry 8 bytes
        payload.assign(4, 0xAB);
    }},
    };
    return cases;
}

//! Find a case by name, nullptr if unknown.
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

inline void FrameTransmitHandler(const CanFrameTransmitEvent& canFrameAck, ILogger* logger)
{
    std::stringstream ss;
    ss << "Receive CAN frame transmit acknowledge: canId=" << canFrameAck.canId << ", status='" << canFrameAck.status
       << "'";
    logger->Info(ss.str());
}

inline void FrameHandler(const CanFrameEvent& canFrameEvent, ILogger* logger, bool printHex)
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
