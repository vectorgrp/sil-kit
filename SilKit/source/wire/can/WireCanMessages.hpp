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
    uint32_t canId; //!< CAN Identifier
    uint32_t flags; //!< CAN Arbitration and Control Field Flags
    //! \brief Data Length Code - describes the length of the dataField
    //! The acceptable bit-patterns and their semantics differ between CAN, CAN FD and CAN XL. The user is responsible
    //! for setting this field correctly. Please consult the CAN specifications for further information.
    uint16_t dlc;
    uint8_t sdt; //!< SDU type - describes the structure of the frames Data Field content (for XL Format only)
    uint8_t vcid; //!< Virtual CAN network ID (for XL Format only)
    uint32_t af; //!< Acceptance field (for XL Format only)
    Util::SharedVector<uint8_t> dataField; //!< The raw CAN data field
};

inline auto ToCanFrame(const WireCanFrame& wireCanFrame) -> CanFrame;
inline auto MakeWireCanFrame(const CanFrame& canFrame) -> WireCanFrame;

/*! \brief //! \brief The event of an incoming CAN frame including transmit ID, timestamp and the actual frame
 */
struct WireCanFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Send time
    WireCanFrame frame; //!< The incoming CAN Frame
    TransmitDirection direction; //!< Receive/Transmit direction
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

inline auto ToCanFrameEvent(const WireCanFrameEvent& wireCanFrameEvent) -> CanFrameEvent;
inline auto MakeWireCanFrameEvent(const CanFrameEvent& canFrameEvent) -> WireCanFrameEvent;

/*! \brief The CAN controller status, sent to the controller
 */
struct CanControllerStatus
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the status change
    CanControllerState controllerState; //!< General State of the CAN controller
    CanErrorState errorState; //!< State of Error Handling
};

/*! \brief The baud rate, sent to the simulator
 */
struct CanConfigureBaudrate
{
    uint32_t baudRate; //!< Specifies the baud rate of the controller in bps (range 0..2000000).
    uint32_t
        fdBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN FD (range 0..16000000).
    uint32_t
        xlBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN XL (range 0..16000000).
};

/*! \brief The CAN controller mode, sent to the simulator
 */
struct CanSetControllerMode
{
    struct Flags
    {
        uint8_t resetErrorHandling : 1; //!< Reset the error counters to zero and the error state to error active.
        uint8_t
            cancelTransmitRequests : 1; //!< Cancel all outstanding transmit requests (flush transmit queue of controller).
    } flags;
    CanControllerState mode; //!< State that the CAN controller should reach.
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
    return {wireCanFrame.canId, wireCanFrame.flags, wireCanFrame.dlc, wireCanFrame.sdt, wireCanFrame.vcid,
            wireCanFrame.af, wireCanFrame.dataField.AsSpan()};
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
