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
    //! The incoming CAN Frame
    SilKit::Services::Can::CanFrame frame;
    //! Optional pointer provided by user when sending the frame
    void* userContext;
};
struct CanConfigureBaudrate
{
    //! Specifies the baud rate of the controller in bps (range 0..2000000).
    uint32_t baudRate;
    //! Specifies the data segment baud rate of the controller in bps for CAN FD (range 0..16000000).
    uint32_t fdBaudRate;
    //! Specifies the data segment baud rate of the controller in bps for CAN XL (range 0..16000000).
    uint32_t xlBaudRate;
};
struct CanControllerMode
{
    //! Flag for resetting the error handling and/or canceling all outstanding transmit requests
    SilKit_Experimental_NetSim_CanControllerModeFlags canControllerModeFlags;
    //! State that the CAN controller should reach.
    SilKit::Services::Can::CanControllerState state;
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
    //! ChiCommand RUN
    RUN = SilKit_FlexrayChiCommand_RUN,
    //! ChiCommand DEFERRED_HALT
    DEFERRED_HALT = SilKit_FlexrayChiCommand_DEFERRED_HALT,
    //! ChiCommand FREEZE
    FREEZE = SilKit_FlexrayChiCommand_FREEZE,
    //! ChiCommand ALLOW_COLDSTART
    ALLOW_COLDSTART = SilKit_FlexrayChiCommand_ALLOW_COLDSTART,
    //! ChiCommand ALL_SLOTS
    ALL_SLOTS = SilKit_FlexrayChiCommand_ALL_SLOTS,
    //! ChiCommand WAKEUP
    WAKEUP = SilKit_FlexrayChiCommand_WAKEUP
};

struct FlexrayHostCommand
{
    FlexrayChiCommand command;
};

} // namespace Flexray

namespace Ethernet {

struct EthernetFrameRequest
{
    //! The Ethernet frame
    SilKit::Services::Ethernet::EthernetFrame ethernetFrame;
    //! Optional pointer provided by user when sending the frame
    void* userContext;
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    //! The controller is inactive (default after reset).
    Inactive = SilKit_EthernetControllerMode_Inactive,
    //! The controller is active.
    Active = SilKit_EthernetControllerMode_Active,
};

struct EthernetControllerMode
{
    //! EthernetMode that the Ethernet controller should reach.
    EthernetMode mode;
};

} // namespace Ethernet

namespace Lin {

struct LinFrameRequest
{
    //! Provide the LIN ID, checksum model, expected data length and optional data.
    SilKit::Services::Lin::LinFrame frame;
    //! Determines whether to provide a frame response or not.
    SilKit::Services::Lin::LinFrameResponseType responseType;
};

struct LinFrameHeaderRequest
{
    //! The LinId of the header to be transmitted
    SilKit::Services::Lin::LinId id;
};

struct LinWakeupPulse
{
    //! Time of the WakeUp pulse. Only valid in detailed Simulation.
    std::chrono::nanoseconds timestamp;
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
    //! Vector of new FrameResponses.
    std::vector<SilKit::Services::Lin::LinFrameResponse> frameResponses;
};

struct LinControllerStatusUpdate
{
    //! Time of the controller status change.
    std::chrono::nanoseconds timestamp;
    //! The new controller status
    SilKit::Services::Lin::LinControllerStatus status;
};

} // namespace Lin

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
