// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/SilKitMacros.hpp"
#include "silkit/capi/NetworkSimulator.h"
#include "silkit/services/datatypes.hpp"
#include "silkit/services/can/CanDatatypes.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/ethernet/EthernetDatatypes.hpp"
#include "silkit/services/lin/LinDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

//!< The identifier of a remote controller.
using ControllerDescriptor = SilKit_Experimental_ControllerDescriptor;

//!< Types of bus networks.
enum class SimulatedNetworkType
{
    Undefined = SilKit_NetworkType_Undefined,
    CAN = SilKit_NetworkType_CAN,
    LIN = SilKit_NetworkType_LIN,
    Ethernet = SilKit_NetworkType_Ethernet,
    FlexRay = SilKit_NetworkType_FlexRay,
};

// --------------------------------
// Can
// --------------------------------

namespace Can {

struct CanFrameRequest
{
    SilKit::Services::Can::CanFrame frame; //!< The incoming CAN Frame
    void* userContext; //!< Optional pointer provided by user when sending the frame
};
struct CanConfigureBaudrate
{
    uint32_t baudRate; //!< Specifies the baud rate of the controller in bps (range 0..2000000).
    uint32_t
        fdBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN FD (range 0..16000000).
    uint32_t
        xlBaudRate; //!< Specifies the data segment baud rate of the controller in bps for CAN XL (range 0..16000000).
};
struct CanControllerMode
{
    SilKit_Experimental_NetSim_CanControllerModeFlags canControllerModeFlags; //!< Flag for resetting the error handling and/or canceling all outstanding transmit requests
    SilKit::Services::Can::CanControllerState state; //!< State that the CAN controller should reach.
};

} // namespace Can

// --------------------------------
// FlexRay
// --------------------------------

namespace Flexray {

//! Update the content of a FlexRay TX-Buffer
struct FlexrayTxBufferUpdate
{
    //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;

    //! Payload data valid flag
    bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    Util::Span<const uint8_t> payload;
};

//! Update the configuration of a particular FlexRay TX-Buffer
struct FlexrayTxBufferConfigUpdate
{
    //! Index of the TX-Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;
    //! The new configuration of the Tx-Buffer
    SilKit::Services::Flexray::FlexrayTxBufferConfig txBufferConfig;
};

//! Configure the communication parameters of the FlexRay controller.
struct FlexrayControllerConfig
{
    //! FlexRay cluster parameters
    SilKit::Services::Flexray::FlexrayClusterParameters clusterParams;
    //! FlexRay node parameters
    SilKit::Services::Flexray::FlexrayNodeParameters nodeParams;
    //! FlexRay buffer configs
    std::vector<SilKit::Services::Flexray::FlexrayTxBufferConfig> bufferConfigs;
};

enum class FlexrayChiCommand : uint8_t
{
    RUN = SilKit_FlexrayChiCommand_RUN, //!< ChiCommand RUN
    DEFERRED_HALT = SilKit_FlexrayChiCommand_DEFERRED_HALT, //!< ChiCommand DEFERRED_HALT
    FREEZE = SilKit_FlexrayChiCommand_FREEZE, //!< ChiCommand FREEZE
    ALLOW_COLDSTART = SilKit_FlexrayChiCommand_ALLOW_COLDSTART, //!< ChiCommand ALLOW_COLDSTART
    ALL_SLOTS = SilKit_FlexrayChiCommand_ALL_SLOTS, //!< ChiCommand ALL_SLOTS
    WAKEUP = SilKit_FlexrayChiCommand_WAKEUP //!< ChiCommand WAKEUP
};

struct FlexrayHostCommand
{
    FlexrayChiCommand command;
};

} // namespace Flexray

namespace Ethernet {

struct EthernetFrameRequest
{
    SilKit::Services::Ethernet::EthernetFrame ethernetFrame; //!< The Ethernet frame
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    Inactive = SilKit_EthernetControllerMode_Inactive, //!< The controller is inactive (default after reset).
    Active = SilKit_EthernetControllerMode_Active, //!< The controller is active.
};

struct EthernetControllerMode
{
    EthernetMode mode; //!< EthernetMode that the Ethernet controller should reach.
};

} // namespace Ethernet

namespace Lin {

struct LinFrameRequest
{
    SilKit::Services::Lin::LinFrame frame; //!< Provide the LIN ID, checksum model, expected data length and optional data.
    SilKit::Services::Lin::LinFrameResponseType responseType; //!< Determines whether to provide a frame response or not.
};

struct LinFrameHeaderRequest
{
    SilKit::Services::Lin::LinId id; //!< The LinId of the header to be transmitted
};

struct LinWakeupPulse
{
    std::chrono::nanoseconds timestamp; //!< Time of the WakeUp pulse. Only valid in detailed Simulation.
};

struct LinControllerConfig
{
    //! Used to configure the simulation mode of the LinController.
    enum class SimulationMode : uint8_t
    {
        //! The LIN controller sets frame responses using SetFrameResponses in advance.
        Default = SilKit_Experimental_NetSim_LinSimulationMode_Default,
        //! The LIN controller does not send frame responses automatically, users must call SendDynamicResponse.
        Dynamic = SilKit_Experimental_NetSim_LinSimulationMode_Dynamic,
    };

    //! Configure as LIN master or LIN slave
    SilKit::Services::Lin::LinControllerMode controllerMode{SilKit::Services::Lin::LinControllerMode::Inactive};
    //! The operational baud rate of the controller. Used in a detailed simulation.
    SilKit::Services::Lin::LinBaudRate baudRate{0};
    //! Optional LinFrameResponse configuration.
    //!
    //! FrameResponses can also be configured at a later point using
    //! ILinController::UpdateTxBuffer() and
    //! ILinController::SetFrameResponses().
    std::vector<SilKit::Services::Lin::LinFrameResponse> frameResponses;

    //! The LinController's simulation mode.
    SimulationMode simulationMode{SimulationMode::Default};
};

struct LinFrameResponseUpdate
{
    std::vector<SilKit::Services::Lin::LinFrameResponse> frameResponses; //!< Vector of new FrameResponses.
};

struct LinControllerStatusUpdate
{
    std::chrono::nanoseconds timestamp; //!< Time of the controller status change.
    SilKit::Services::Lin::LinControllerStatus status; //!< The new controller status
};

} // namespace Lin

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
