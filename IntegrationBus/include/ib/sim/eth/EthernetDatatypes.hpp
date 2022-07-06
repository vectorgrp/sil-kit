// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "silkit/SilKitMacros.hpp"
#include "silkit/services/datatypes.hpp"

// ================================================================================
//  Ethernet specific data types
// ================================================================================
namespace SilKit {
namespace Services {
namespace Ethernet {

//! \brief An Ethernet MAC address, i.e. FF:FF:FF:FF:FF:FF
using EthernetMac = std::array<uint8_t, 6>;

//! \brief Bitrate in kBit/sec
using EthernetBitrate = uint32_t;

//! \brief An Ethernet frame (layer 2)
struct EthernetFrame
{
    std::vector<uint8_t> raw; //!< The Ethernet raw frame without the frame check sequence
};

////////////////////////////////////////////////////////////////////////////////
// EthernetFrame Inline definitions
////////////////////////////////////////////////////////////////////////////////

//! \brief An Ethernet transmit id
using EthernetTxId = uint32_t;

//! \brief An Ethernet frame including the raw frame, Transmit ID and timestamp
struct EthernetFrameEvent
{
    EthernetTxId transmitId; //!< Set by the EthController, used for acknowledgments
    std::chrono::nanoseconds timestamp; //!< Reception time
    EthernetFrame frame; //!< The Ethernet frame
};

//! \brief Acknowledgment status for an EthernetTransmitRequest
enum class EthernetTransmitStatus : uint8_t
{
    /*! The message was successfully transmitted on the Ethernet link.
    */
    Transmitted = 0,

    /*! The transmit request was rejected, because the Ethernet controller is not active.
    */
    ControllerInactive = 1,

    /*! The transmit request was rejected, because the Ethernet link is down.
    */
    LinkDown = 2,

    /*! The transmit request was dropped, because the transmit queue is full.
    */
    Dropped = 3,

    /*! (currently not in use)
     *
     * The transmit request was rejected, because there is already another request with the same transmitId.
    */
    DuplicatedTransmitId = 4,

    /*! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, wrong order of VLAN tags).
    */
    InvalidFrameFormat = 5
};

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetFrameTransmitEvent
{
    EthernetTxId transmitId;   //!< Identifies the EthernetTransmitRequest, to which this EthernetFrameTransmitEvent refers to.
    EthernetMac sourceMac; //!< The source MAC address encoded as integral data type
    std::chrono::nanoseconds timestamp; //!< Timestamp of the Ethernet acknowledge.
    EthernetTransmitStatus status; //!< Status of the EthernetTransmitRequest.
};

//! \brief State of the Ethernet controller
enum class EthernetState : uint8_t
{
    Inactive = 0, //!< The Ethernet controller is switched off (default after reset).
    LinkDown = 1, //!< The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
    LinkUp = 2, //!< The Ethernet controller is active and the link to another Ethernet controller is established.
};

//! \brief A state change event of the Ethernet controller
struct EthernetStateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    EthernetState state; //!< State of the Ethernet controller.
};

//! \brief A bitrate change event of the Ethernet controller
struct EthernetBitrateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    EthernetBitrate bitrate; //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
};

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetStatus
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the status change.
    EthernetState state; //!< State of the Ethernet controller.
    EthernetBitrate bitrate; //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    Inactive = 0, //!< The controller is inactive (default after reset).
    Active = 1, //!< The controller is active.
};

//! \brief Set the Mode of the Ethernet Controller.
struct EthernetSetMode
{
    EthernetMode mode; //!< EthernetMode that the Ethernet controller should reach.
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
