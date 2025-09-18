// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/can/string_utils.hpp"

#include "SharedVector.hpp"

#include <chrono>
#include <vector>

namespace SilKit {
namespace Services {
namespace Can {

struct WireCanFrame
{
    // CAN frame content
//! CAN Identifier
    uint32_t canId;
//! CAN Arbitration and Control Field Flags
    uint32_t flags;
    //! \brief Data Length Code - describes the length of the dataField
    //! The acceptable bit-patterns and their semantics differ between CAN, CAN FD and CAN XL. The user is responsible
    //! for setting this field correctly. Please consult the CAN specifications for further information.
    uint16_t dlc;
//! SDU type - describes the structure of the frames Data Field content (for XL Format only)
    uint8_t sdt; 
//! Virtual CAN network ID (for XL Format only)
    uint8_t vcid;
//! Acceptance field (for XL Format only)
    uint32_t af; 
//! The raw CAN data field
    Util::SharedVector<uint8_t> dataField;
};

inline auto ToCanFrame(const WireCanFrame& wireCanFrame) -> CanFrame;
inline auto MakeWireCanFrame(const CanFrame& canFrame) -> WireCanFrame;

/*! \brief //! \brief The event of an incoming CAN frame including transmit ID, timestamp and the actual frame
 */
struct WireCanFrameEvent
{
//! Send time
    std::chrono::nanoseconds timestamp;
//! The incoming CAN Frame
    WireCanFrame frame;                
//! Receive/Transmit direction
    TransmitDirection direction;       
//! Optional pointer provided by user when sending the frame
    void* userContext;                 
};

inline auto ToCanFrameEvent(const WireCanFrameEvent& wireCanFrameEvent) -> CanFrameEvent;
inline auto MakeWireCanFrameEvent(const CanFrameEvent& canFrameEvent) -> WireCanFrameEvent;

/*! \brief The CAN controller status, sent to the controller
 */
struct CanControllerStatus
{
//! Timestamp of the status change
    std::chrono::nanoseconds timestamp;
//! General State of the CAN controller
    CanControllerState controllerState;
//! State of Error Handling
    CanErrorState errorState;          
};

/*! \brief The baud rate, sent to the simulator
 */
struct CanConfigureBaudrate
{
//! Specifies the baud rate of the controller in bps (range 0..2000000).
    uint32_t baudRate;
//! Specifies the data segment baud rate of the controller in bps for CAN FD (range 0..16000000).
    uint32_t fdBaudRate;
//! Specifies the data segment baud rate of the controller in bps for CAN XL (range 0..16000000).
    uint32_t xlBaudRate;
};

/*! \brief The CAN controller mode, sent to the simulator
 */
struct CanSetControllerMode
{
    struct Flags
    {
//! Reset the error counters to zero and the error state to error active.
        uint8_t resetErrorHandling : 1;
//! Cancel all outstanding transmit requests (flush transmit queue of controller).
        uint8_t cancelTransmitRequests : 1;
    } flags;
//! State that the CAN controller should reach.
    CanControllerState mode;
};

inline std::string to_string(const WireCanFrame& msg);
inline std::string to_string(const CanControllerStatus& status);
inline std::string to_string(const CanConfigureBaudrate& rate);
inline std::string to_string(const CanSetControllerMode& mode);

inline std::ostream& operator<<(std::ostream& out, const WireCanFrameEvent& msg);
inline std::ostream& operator<<(std::ostream& out, const CanControllerStatus& status);
inline std::ostream& operator<<(std::ostream& out, const CanConfigureBaudrate& rate);
inline std::ostream& operator<<(std::ostream& out, const CanSetControllerMode& mode);

// ================================================================================
//  Inline Implementations
// ================================================================================

auto ToCanFrame(const WireCanFrame& wireCanFrame) -> CanFrame
{
    return {wireCanFrame.canId,
            wireCanFrame.flags,
            wireCanFrame.dlc,
            wireCanFrame.sdt,
            wireCanFrame.vcid,
            wireCanFrame.af,
            wireCanFrame.dataField.AsSpan()};
}

auto MakeWireCanFrame(const CanFrame& canFrame) -> WireCanFrame
{
    return {canFrame.canId, canFrame.flags, canFrame.dlc, canFrame.sdt, canFrame.vcid, canFrame.af, canFrame.dataField};
}

auto ToCanFrameEvent(const WireCanFrameEvent& wireCanFrameEvent) -> CanFrameEvent
{
    return {wireCanFrameEvent.timestamp, ToCanFrame(wireCanFrameEvent.frame), wireCanFrameEvent.direction,
            wireCanFrameEvent.userContext};
}

auto MakeWireCanFrameEvent(const CanFrameEvent& canFrameEvent) -> WireCanFrameEvent
{
    return {canFrameEvent.timestamp, MakeWireCanFrame(canFrameEvent.frame), canFrameEvent.direction,
            canFrameEvent.userContext};
}

std::string to_string(const WireCanFrame& msg)
{
    return to_string(ToCanFrame(msg));
}

std::string to_string(const CanControllerStatus& status)
{
    std::stringstream outStream;
    outStream << status;
    return outStream.str();
}

std::string to_string(const CanConfigureBaudrate& rate)
{
    std::stringstream outStream;
    outStream << rate;
    return outStream.str();
}

std::string to_string(const CanSetControllerMode& mode)
{
    std::stringstream outStream;
    outStream << mode;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const WireCanFrameEvent& msg)
{
    return out << ToCanFrameEvent(msg);
}

std::ostream& operator<<(std::ostream& out, const CanControllerStatus& status)
{
    auto timestamp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(status.timestamp);
    return out << "can::CanControllerStatus{CtrlState=" << status.controllerState
               << ", ErrorState=" << status.errorState << " @" << timestamp.count() << "ms}";
}

std::ostream& operator<<(std::ostream& out, const CanConfigureBaudrate& rate)
{
    return out << "can::CanConfigureBaudrate{" << rate.baudRate << ", " << rate.fdBaudRate << "}";
}

std::ostream& operator<<(std::ostream& out, const CanSetControllerMode& mode)
{
    out << "can::CanSetControllerMode{" << mode.mode;

    if (mode.flags.cancelTransmitRequests)
        out << ", cancelTX";
    if (mode.flags.resetErrorHandling)
        out << ", resetErrorHandling";
    out << "}";
    return out;
}

} // namespace Can
} // namespace Services
} // namespace SilKit
