// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
//! Identifier Extension
    Ide = SilKit_CanFrameFlag_ide,
//! Remote Transmission Request
    Rtr = SilKit_CanFrameFlag_rtr,
//! FD Format Indicator
    Fdf = SilKit_CanFrameFlag_fdf,
//! Bit Rate Switch  (for FD Format only)
    Brs = SilKit_CanFrameFlag_brs,
//! Error State indicator (for FD Format only)
    Esi = SilKit_CanFrameFlag_esi,
//! XL Format Indicator
    Xlf = SilKit_CanFrameFlag_xlf,
//! Simple Extended Content (for XL Format only)
    Sec = SilKit_CanFrameFlag_sec,
};

/*! \brief A CAN Frame
 */
struct CanFrame
{
    // CAN frame content
//! CAN Identifier
    uint32_t canId;        
//! CAN Arbitration and Control Field Flags
    CanFrameFlagMask flags;
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
    Util::Span<const uint8_t> dataField;
};

/*! \brief The event of an incoming CAN frame including transmit ID, timestamp and the actual frame
 */
struct CanFrameEvent
{
//! Send time
    std::chrono::nanoseconds timestamp;
//! The incoming CAN Frame
    CanFrame frame;                    
//! Receive/Transmit direction
    TransmitDirection direction;       
//! Optional pointer provided by user when sending the frame
    void* userContext;                 
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
//! Identifies the CAN id to which this CanFrameTransmitEvent refers to.
    uint32_t canId;                    
//! Timestamp of the CAN acknowledge.
    std::chrono::nanoseconds timestamp;
//! Status of the CanTransmitRequest.
    CanTransmitStatus status;          
//! Optional pointer provided by user when sending the frame
    void* userContext;                 
};

/*! \brief An incoming state change event
 */
struct CanStateChangeEvent
{
//! Timestamp of the state change.
    std::chrono::nanoseconds timestamp;
//! The new state
    CanControllerState state;          
};

/*! \brief An incoming error state change event
 */
struct CanErrorStateChangeEvent
{
//! Timestamp of the state change.
    std::chrono::nanoseconds timestamp;
//! The new error state
    CanErrorState errorState;          
};

} // namespace Can
} // namespace Services
} // namespace SilKit
