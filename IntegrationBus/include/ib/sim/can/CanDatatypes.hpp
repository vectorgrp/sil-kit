//!< Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

// ================================================================================
//  CAN specific data types
// ================================================================================
namespace ib {
namespace sim {
namespace can {

using CanTxId = uint32_t;

/*! \brief A CAN message, sent in both directions
 * 
 * Directions:
 * - From: Controller To: Simulator
 * - From: Simulator  To: Controller
 */
struct CanMessage
{
    // Meta Data
    CanTxId transmitId; //!< Set by the CanController, used for acknowledgements
    std::chrono::nanoseconds timestamp; //!< Reception time; set by can Network Simulator

    // CAN message content
    uint32_t canId;
    struct CanReceiveFlags
    {
        uint8_t ide : 1; //!< Identifier Extension
        uint8_t rtr : 1; //!< Remote Transmission Request
        uint8_t fdf : 1; //!< FD Format Indicator
        uint8_t brs : 1; //!< Bit Rate Switch  (for FD Format only)
        uint8_t esi : 1; //!< Error State indicator (for FD Format only)
    } flags;
    uint8_t dlc : 4; //!< Data Length Code - determined by the Network Simulator
    std::vector<uint8_t> dataField;
};

/*! \brief CAN Controller state according to AUTOSAR specification AUTOSAR_SWS_CANDriver 4.3.1
 */
enum class CanControllerState : uint8_t
{
  Uninit = 0,  //!< CAN controller is not initialized (initial state after reset)
  Stopped = 1, //!< CAN controller is initialized, but not participating on the CAN bus
  Started = 2, //!< CAN controller is in normal operation mode
  Sleep = 3,   //!< CAN controller is in sleep mode.
};
/*! \brief Error state of a CAN node according to CAN specification
 */
enum class CanErrorState : uint8_t
{
  NotAvailable = 0, //!< Error State is not available, because can controller is in state Uninit.
  ErrorActive = 1,  //!< Error Active Mode, the CAN controller is allowed to send messages and active error flags.
  ErrorPassive = 2, //!< Error passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags
  BusOff = 3,       //!< Bus Off Mode, the CAN controller must not have any influence on the bus. It can neither send messages nor active error flags
};

/*! \brief The CAN controller status, sent to the controller
 * 
 * Directions:
 * - From: Simulator  To: Controller
 */
struct CanControllerStatus
{
    std::chrono::nanoseconds timestamp; //!< Set by Network Simulator
    CanControllerState controllerState; //!< General State of the CAN controller
    CanErrorState errorState; //!< State of Error Handling
};
/*! \brief Transmit state of a CAN node according to CAN specification
 */
enum class CanTransmitStatus : uint8_t
{
  Transmitted = 0, //!< The message was successfully transmitted on the CAN bus.
  Canceled = 1, //!< The transmit queue was reset (see CanSetControllerMode field cancelTransmitRequests)
  TransmitQueueFull = 2, //!< The transmit request was rejected, because the transmit queue is full.
  DuplicatedTransmitId = 3, //!< The  transmit request was rejected, because there is already another request with the same transmitId
};

/*! \brief The acknowledgment of a CAN message, sent to the controller
 * 
 * Directions:
 * - From: Simulator  To: Controller
 */
struct CanTransmitAcknowledge
{
    CanTxId transmitId; //!< Identifies the CanTransmitRequest to which this CanTransmitAcknowledge refers to
    std::chrono::nanoseconds timestamp; //!< Set by Network Simulator
    CanTransmitStatus status; //!< Status of the CAN Transmit Request
};

/*! \brief The baud rate, sent to the simulator
 * 
 * Directions:
 * - From: Controller  To: Simulator
 */
struct CanConfigureBaudrate
{
    uint32_t baudRate;   //!< Specifies the baud rate of the controller in bps (range 0..2000000)
    uint32_t fdBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN FD(range 0..16000000)
};

/*! \brief The CAN controller mode, sent to the simulator
 * 
 * Directions:
 * - From: Controller  To: Simulator
 */
struct CanSetControllerMode
{
    struct Flags
    {
        uint8_t resetErrorHandling : 1; //!< Reset the error counters to zero and the error state to error active.
        uint8_t cancelTransmitRequests : 1; //!< Cancel all outstanding transmit requests (flush transmit queue of controller).
    } flags;
    CanControllerState mode; //!< State that the CAN controller should reach
};

} // namespace can
} // namespace sim
} // namespace ib
