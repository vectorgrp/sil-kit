// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

// ================================================================================
//  CAN specific data types
// ================================================================================
namespace ib {
namespace sim {
//! The CAN namespace
namespace can {

using CanTxId = uint32_t;

/*! \brief A CAN message
 */
struct CanMessage
{
    // Meta Data
    CanTxId transmitId; //!< Set by the CanController, used for acknowledgements
    std::chrono::nanoseconds timestamp; //!< Reception time

    // CAN message content
    uint32_t canId; //!< CAN Identifier
    struct CanReceiveFlags
    {
        uint8_t ide : 1; //!< Identifier Extension
        uint8_t rtr : 1; //!< Remote Transmission Request
        uint8_t fdf : 1; //!< FD Format Indicator
        uint8_t brs : 1; //!< Bit Rate Switch  (for FD Format only)
        uint8_t esi : 1; //!< Error State indicator (for FD Format only)
    } flags; //!< CAN Arbitration and Control Field Flags
    uint8_t dlc : 4; //!< Data Length Code - determined by the Network Simulator
    std::vector<uint8_t> dataField; //!< CAN Datafield
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
/*! \brief Transmit state of a CAN node according to CAN specification
 */
enum class CanTransmitStatus : uint8_t
{
  /*! The message was successfully transmitted on the CAN bus.
  */
  Transmitted = 0,

  /*! (currently not in use)
   *
   * The transmit queue was reset.
  */
  Canceled = 1,

  /*! The transmit request was rejected, because the transmit queue is full.
  */
  TransmitQueueFull = 2,

  /*! (currently not in use)
   *
   * The transmit request was rejected, because there is already another request with the same transmitId.
  */
  DuplicatedTransmitId = 3,
};

/*! \brief The acknowledgment of a CAN message, sent to the controller
 */
struct CanTransmitAcknowledge
{
    CanTxId transmitId; //!< Identifies the CanTransmitRequest to which this CanTransmitAcknowledge refers to.
    uint32_t canId; //!< Identifies the CAN id to which this CanTransmitAcknowledge refers to.
    std::chrono::nanoseconds timestamp; //!< Timestamp of the CAN acknowledge.
    CanTransmitStatus status; //!< Status of the CanTransmitRequest.
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
    struct Flags
    {
        uint8_t resetErrorHandling : 1; //!< Reset the error counters to zero and the error state to error active.
        uint8_t cancelTransmitRequests : 1; //!< Cancel all outstanding transmit requests (flush transmit queue of controller).
    } flags;
    CanControllerState mode; //!< State that the CAN controller should reach.
};

} // namespace can
} // namespace sim
} // namespace ib
