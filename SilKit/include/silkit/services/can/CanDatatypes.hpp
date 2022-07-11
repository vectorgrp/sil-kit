// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "silkit/services/datatypes.hpp"

// ================================================================================
//  CAN specific data types
// ================================================================================
namespace SilKit {
namespace Services {
namespace Can {

using CanTxId = uint32_t; //!< Set by the CanController, used for acknowledgements

/*! \brief A CAN Frame
 */
struct CanFrame
{
    // CAN frame content
    uint32_t canId; //!< CAN Identifier
    struct CanFrameFlags
    {
        uint8_t ide : 1; //!< Identifier Extension
        uint8_t rtr : 1; //!< Remote Transmission Request
        uint8_t fdf : 1; //!< FD Format Indicator
        uint8_t brs : 1; //!< Bit Rate Switch  (for FD Format only)
        uint8_t esi : 1; //!< Error State indicator (for FD Format only)
    } flags; //!< CAN Arbitration and Control Field Flags
    uint8_t dlc : 4; //!< Data Length Code - determined by a network simulator
    std::vector<uint8_t> dataField; //!< The raw CAN data field
};

/*! \brief The event of an incoming CAN frame including transmit ID, timestamp and the actual frame
 */
struct CanFrameEvent
{
    CanTxId transmitId; //!< Set by the CanController, used for acknowledgements
    std::chrono::nanoseconds timestamp; //!< Send time
    CanFrame frame; //!< The incoming CAN Frame
    TransmitDirection direction{TransmitDirection::Undefined}; //!< Receive/Transmit direction
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

/*! \brief The CAN controller status, sent to the controller
 */
struct CanControllerStatus
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the status change
    CanControllerState controllerState; //!< General State of the CAN controller
    CanErrorState errorState; //!< State of Error Handling
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
    CanTxId transmitId; //!< Identifies the CanTransmitRequest to which this CanFrameTransmitEvent refers to.
    uint32_t canId; //!< Identifies the CAN id to which this CanFrameTransmitEvent refers to.
    std::chrono::nanoseconds timestamp; //!< Timestamp of the CAN acknowledge.
    CanTransmitStatus status; //!< Status of the CanTransmitRequest.
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

/*! \brief The baud rate, sent to the simulator
 */
struct CanConfigureBaudrate
{
    uint32_t baudRate;   //!< Specifies the baud rate of the controller in bps (range 0..2000000).
    uint32_t fdBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN FD(range 0..16000000).
};

/*! \brief The CAN controller mode, sent to the simulator
 */
struct CanSetControllerMode
{
    struct ControllerModeFlag
    {
        uint8_t resetErrorHandling : 1; //!< Reset the error counters to zero and the error state to error active.
        uint8_t cancelTransmitRequests : 1; //!< Cancel all outstanding transmit requests (flush transmit queue of controller).
    } flags;
    CanControllerState mode; //!< State that the CAN controller should reach.
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