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

#include <chrono>

#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

#include "silkit/capi/Can.h"

// ================================================================================
//  CAN specific data types
// ================================================================================

namespace SilKit {
namespace Services {
namespace Can {

using CanFrameFlagMask = SilKit_CanFrameFlag;

enum class CanFrameFlag : CanFrameFlagMask
{
    Ide = SilKit_CanFrameFlag_ide, //!< Identifier Extension
    Rtr = SilKit_CanFrameFlag_rtr, //!< Remote Transmission Request
    Fdf = SilKit_CanFrameFlag_fdf, //!< FD Format Indicator
    Brs = SilKit_CanFrameFlag_brs, //!< Bit Rate Switch  (for FD Format only)
    Esi = SilKit_CanFrameFlag_esi, //!< Error State indicator (for FD Format only)
    Xlf = SilKit_CanFrameFlag_xlf, //!< XL Format Indicator
    Sec = SilKit_CanFrameFlag_sec, //!< Simple Extended Content (for XL Format only)
};

/*! \brief A CAN Frame
 */
struct CanFrame
{
    // CAN frame content
    uint32_t canId; //!< CAN Identifier
    CanFrameFlagMask flags; //!< CAN Arbitration and Control Field Flags
    //! \brief Data Length Code - describes the length of the dataField
    //! The acceptable bit-patterns and their semantics differ between CAN, CAN FD and CAN XL. The user is responsible
    //! for setting this field correctly. Please consult the CAN specifications for further information.
    uint16_t dlc;
    uint8_t sdt; //!< SDU type - describes the structure of the frames Data Field content (for XL Format only)
    uint8_t vcid; //!< Virtual CAN network ID (for XL Format only)
    uint32_t af; //!< Acceptance field (for XL Format only)
    Util::Span<const uint8_t> dataField; //!< The raw CAN data field
};

/*! \brief The event of an incoming CAN frame including transmit ID, timestamp and the actual frame
 */
struct CanFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Send time
    CanFrame frame; //!< The incoming CAN Frame
    TransmitDirection direction; //!< Receive/Transmit direction
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

/*! \brief CAN Controller state according to AUTOSAR specification AUTOSAR_SWS_CANDriver 4.3.1
 */
enum class CanControllerState : SilKit_CanControllerState
{
  //! CAN controller is not initialized (initial state after reset).
  Uninit = SilKit_CanControllerState_Uninit,

  //! CAN controller is initialized but does not participate on the CAN bus.
  Stopped = SilKit_CanControllerState_Stopped,

  //! CAN controller is in normal operation mode.
  Started = SilKit_CanControllerState_Started,

  //! CAN controller is in sleep mode which is similar to the Stopped state.
  Sleep = SilKit_CanControllerState_Sleep,
};


/*! \brief Error state of a CAN node according to CAN specification.
 */
enum class CanErrorState : SilKit_CanErrorState
{
  /*! Error State is Not Available, because CAN controller is in state Uninit.
  *
  * *AUTOSAR Doc:* Successful transmission.
  */
  NotAvailable = SilKit_CanErrorState_NotAvailable,

  /*! Error Active Mode, the CAN controller is allowed to send messages and active error flags.
  */
  ErrorActive = SilKit_CanErrorState_ErrorActive,

  /*! Error Passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags.
  */
  ErrorPassive = SilKit_CanErrorState_ErrorPassive,

  /*! (currently not in use)
   *
   * *AUTOSAR Doc:* Bus Off Mode, the CAN controller does not take part in communication.
  */
  BusOff = SilKit_CanErrorState_BusOff,
};

using CanTransmitStatusMask = SilKit_CanTransmitStatus;

/*! \brief Transfer status of a CAN node according to CAN specification
 */
enum class CanTransmitStatus : CanTransmitStatusMask
{
    //! The message was successfully transmitted on the CAN bus.
    Transmitted = SilKit_CanTransmitStatus_Transmitted,

    //! The transmit queue was reset. (currently not in use)
    Canceled = SilKit_CanTransmitStatus_Canceled,

    //! The transmit request was rejected, because the transmit queue is full.
    TransmitQueueFull = SilKit_CanTransmitStatus_TransmitQueueFull,
};

/*! \brief The acknowledgment of a CAN message, sent to the controller
 */
struct CanFrameTransmitEvent
{
    uint32_t canId; //!< Identifies the CAN id to which this CanFrameTransmitEvent refers to.
    std::chrono::nanoseconds timestamp; //!< Timestamp of the CAN acknowledge.
    CanTransmitStatus status; //!< Status of the CanTransmitRequest.
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

/*! \brief An incoming state change event
 */
struct CanStateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    CanControllerState state; //!< The new state
};

/*! \brief An incoming error state change event
 */
struct CanErrorStateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    CanErrorState errorState; //!< The new error state
};

} // namespace Can
} // namespace Services
} // namespace SilKit
