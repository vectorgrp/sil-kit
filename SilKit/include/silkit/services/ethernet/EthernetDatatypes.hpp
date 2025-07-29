// Copyright (c) 2022 Vector Informatik GmbH
// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <chrono>

#include "silkit/SilKitMacros.hpp"
#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

#include "silkit/capi/Ethernet.h"

// ================================================================================
//  Ethernet specific data types
// ================================================================================

namespace SilKit {
namespace Services {
namespace Ethernet {

//! \brief Bitrate in kBit/sec
using EthernetBitrate = uint32_t;

//! \brief An Ethernet frame (layer 2)
struct EthernetFrame
{
    Util::Span<const uint8_t> raw; //!< The Ethernet raw frame without the frame check sequence
};

//! \brief An Ethernet frame including the raw frame, Transmit ID and timestamp
struct EthernetFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Reception time
    EthernetFrame frame; //!< The Ethernet frame
    TransmitDirection direction; //!< Receive/Transmit direction
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

using EthernetTransmitStatusMask = SilKit_EthernetTransmitStatus;

//! \brief Acknowledgment status for an EthernetTransmitRequest
enum class EthernetTransmitStatus : EthernetTransmitStatusMask
{
    //! The message was successfully transmitted on the Ethernet link.
    Transmitted = SilKit_EthernetTransmitStatus_Transmitted,

    //! The transmit request was rejected, because the Ethernet controller is not active.
    ControllerInactive = SilKit_EthernetTransmitStatus_ControllerInactive,

    //! The transmit request was rejected, because the Ethernet link is down.
    LinkDown = SilKit_EthernetTransmitStatus_LinkDown,

    //! The transmit request was dropped, because the transmit queue is full.
    Dropped = SilKit_EthernetTransmitStatus_Dropped,

    //! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, wrong order of VLAN tags).
    InvalidFrameFormat = SilKit_EthernetTransmitStatus_InvalidFrameFormat,
};

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetFrameTransmitEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the Ethernet acknowledge.
    EthernetTransmitStatus status; //!< Status of the EthernetTransmitRequest.
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

//! \brief State of the Ethernet controller
enum class EthernetState : SilKit_EthernetState
{
    //! The Ethernet controller is switched off (default after reset).
    Inactive = SilKit_EthernetState_Inactive,
    //! The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
    LinkDown = SilKit_EthernetState_LinkDown,
    //! The Ethernet controller is active and the link to another Ethernet controller is established.
    LinkUp = SilKit_EthernetState_LinkUp,
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

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
