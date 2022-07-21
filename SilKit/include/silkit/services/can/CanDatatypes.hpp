// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>

#include "silkit/capi/Can.h"
#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

// ================================================================================
//  CAN specific data types
// ================================================================================
namespace SilKit {
namespace Services {
namespace Can {

using CanFrameFlagMask = uint32_t;

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
enum class CanControllerState : uint8_t
{
  /*! CAN controller is not initialized (initial state after reset).
  */
  Uninit = 0,

  /*! CAN controller is initialized but does not participate on the CAN bus.
  */
  Stopped = 1,

  /*! CAN controller is in normal operation mode.
  */
  Started = 2,

  /*! CAN controller is in sleep mode which is similar to the Stopped state.
  */
  Sleep = 3,
};


/*! \brief Error state of a CAN node according to CAN specification.
 */
enum class CanErrorState : uint8_t
{
  /*! Error State is Not Available, because CAN controller is in state Uninit.
  *
  * *AUTOSAR Doc:* Successful transmission.
  */
  NotAvailable = 0,

  /*! Error Active Mode, the CAN controller is allowed to send messages and active error flags.
  */
  ErrorActive = 1,

  /*! Error Passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags.
  */
  ErrorPassive = 2,

  /*! (currently not in use)
   *
   * *AUTOSAR Doc:* Bus Off Mode, the CAN controller does not take part in communication.
  */
  BusOff = 3,
};

using  CanTransmitStatusMask = uint16_t;

/*! \brief Transfer status of a CAN node according to CAN specification
 */
enum class CanTransmitStatus : CanTransmitStatusMask
{
    /*! The message was successfully transmitted on the CAN bus.
    */
    Transmitted = (CanTransmitStatusMask)(1 << 0),

    /*! (currently not in use)
     *
     * The transmit queue was reset.
    */
    Canceled = (CanTransmitStatusMask)(1 << 1),

    /*! The transmit request was rejected, because the transmit queue is full.
    */
    TransmitQueueFull = (CanTransmitStatusMask)(1 << 2),

    /*! (currently not in use)
     *
     * The transmit request was rejected, because there is already another request with the same transmitId.
    */
    DuplicatedTransmitId = (CanTransmitStatusMask)(1 << 3)
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
