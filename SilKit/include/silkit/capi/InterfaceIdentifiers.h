// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <limits.h>
#include <stdint.h>
#include <memory.h> //memset
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

SILKIT_BEGIN_DECLS

typedef struct
{
    uint64_t version; //!< Version encoded using SK_ID_MAKE
    uint64_t _reserved[3]; //!< For future expansions
} SilKit_StructHeader;


//! The SK_ID_ macros are internal helpers. They are not to be used directly.
//Service Ids
#define SK_ID_SERVICE_START 0 //!< sentinel
#define SK_ID_SERVICE_Can 1
#define SK_ID_SERVICE_Ethernet 2
#define SK_ID_SERVICE_Flexray 3
#define SK_ID_SERVICE_Lin 4
#define SK_ID_SERVICE_Data 5
#define SK_ID_SERVICE_Rpc 6
#define SK_ID_SERVICE_Participant 7
#define SK_ID_SERVICE_NetworkSimulation 8
#define SK_ID_SERVICE_END 9 //!< sentinel

#define SK_INVALID_DATATYPE_ID 0
#define SK_INVALID_VERSION 0


//! The identifiers service id
#define SK_ID_GET_SERVICE(ID) ((ID >> 40) & 0xFF)
//! The identifiers data type id
#define SK_ID_GET_DATATYPE(ID) ((ID >> 32) & 0xFF)
//! The identifiers data type version
#define SK_ID_GET_VERSION(ID) ((ID >> 24) & 0xFF)

#define SK_ID_MAKE(SERVICE_NAME, DATATYPE_NAME) \
    (uint64_t)(((uint64_t)83 /*S*/ << 56) | ((uint64_t)75 /*K*/ << 48) \
               | ((uint64_t)(SK_ID_SERVICE_##SERVICE_NAME & 0xFF) << 40) \
               | ((uint64_t)(DATATYPE_NAME##_DATATYPE_ID & 0xff) << 32) \
               | ((uint64_t)(DATATYPE_NAME##_VERSION & 0xff) << 24) | 0x0)

#define SK_ID_IS_VALID(ID) \
    ((SK_ID_GET_SERVICE(ID) > SK_ID_SERVICE_START) && (SK_ID_GET_SERVICE(ID) < SK_ID_SERVICE_END) \
     && (SK_ID_GET_DATATYPE(ID) != SK_INVALID_DATATYPE_ID) && (SK_ID_GET_VERSION(ID) != SK_INVALID_VERSION) \
     && ((ID >> 56 & 0xff) == /*S*/ 83) && ((ID >> 48 & 0xff) == /*K*/ 75))

//! Helper to access the Interface ID member
#define SilKit_Struct_GetHeader(VALUE) ((VALUE).structHeader)
//! Helper to access the Interface ID member
#define SilKit_Struct_GetId(VALUE) (SilKit_Struct_GetHeader(VALUE).version)
//! Initialize the struct VALUE with a valid type specific struct header for the given type DATATYPE
#define SilKit_Struct_Init(DATATYPE, VALUE) \
    do \
    { \
        memset(&(VALUE), 0x0, sizeof(VALUE)); \
        SilKit_Struct_GetId(VALUE) = DATATYPE##_STRUCT_VERSION; \
    } while (0)

// CAN
// CAN data type IDs
#define SilKit_CanFrame_DATATYPE_ID 1
#define SilKit_CanFrameTransmitEvent_DATATYPE_ID 2
#define SilKit_CanFrameEvent_DATATYPE_ID 3
#define SilKit_CanStateChangeEvent_DATATYPE_ID 4
#define SilKit_CanErrorStateChangeEvent_DATATYPE_ID 5

// CAN data type versions
#define SilKit_CanFrame_VERSION 1
#define SilKit_CanFrameTransmitEvent_VERSION 2
#define SilKit_CanFrameEvent_VERSION 1
#define SilKit_CanStateChangeEvent_VERSION 1
#define SilKit_CanErrorStateChangeEvent_VERSION 1

// CAN make versioned IDs
#define SilKit_CanFrame_STRUCT_VERSION SK_ID_MAKE(Can, SilKit_CanFrame)
#define SilKit_CanFrameTransmitEvent_STRUCT_VERSION SK_ID_MAKE(Can, SilKit_CanFrameTransmitEvent)
#define SilKit_CanFrameEvent_STRUCT_VERSION SK_ID_MAKE(Can, SilKit_CanFrameEvent)
#define SilKit_CanStateChangeEvent_STRUCT_VERSION SK_ID_MAKE(Can, SilKit_CanStateChangeEvent)
#define SilKit_CanErrorStateChangeEvent_STRUCT_VERSION SK_ID_MAKE(Can, SilKit_CanErrorStateChangeEvent)

// Ethernet
//
// Ethernet data type IDs
#define SilKit_EthernetFrameEvent_DATATYPE_ID 1
#define SilKit_EthernetFrameTransmitEvent_DATATYPE_ID 2
#define SilKit_EthernetStateChangeEvent_DATATYPE_ID 3
#define SilKit_EthernetBitrateChangeEvent_DATATYPE_ID 4
#define SilKit_EthernetFrame_DATATYPE_ID 5

// Ethernet data type versions
#define SilKit_EthernetFrameEvent_VERSION 1
#define SilKit_EthernetFrameTransmitEvent_VERSION 1
#define SilKit_EthernetStateChangeEvent_VERSION 1
#define SilKit_EthernetBitrateChangeEvent_VERSION 1
#define SilKit_EthernetFrame_VERSION 1

#define SilKit_EthernetFrameEvent_STRUCT_VERSION SK_ID_MAKE(Ethernet, SilKit_EthernetFrameEvent)
#define SilKit_EthernetFrameTransmitEvent_STRUCT_VERSION SK_ID_MAKE(Ethernet, SilKit_EthernetFrameTransmitEvent)
#define SilKit_EthernetStateChangeEvent_STRUCT_VERSION SK_ID_MAKE(Ethernet, SilKit_EthernetStateChangeEvent)
#define SilKit_EthernetBitrateChangeEvent_STRUCT_VERSION SK_ID_MAKE(Ethernet, SilKit_EthernetBitrateChangeEvent)
#define SilKit_EthernetFrame_STRUCT_VERSION SK_ID_MAKE(Ethernet, SilKit_EthernetFrame)

// FlexRay
// FlexRay data type IDs
#define SilKit_FlexrayFrameEvent_DATATYPE_ID 1
#define SilKit_FlexrayFrameTransmitEvent_DATATYPE_ID 2
#define SilKit_FlexraySymbolEvent_DATATYPE_ID 3
#define SilKit_FlexraySymbolTransmitEvent_DATATYPE_ID 5
#define SilKit_FlexrayCycleStartEvent_DATATYPE_ID 6
#define SilKit_FlexrayPocStatusEvent_DATATYPE_ID 7
#define SilKit_FlexrayWakeupEvent_DATATYPE_ID 8
#define SilKit_FlexrayControllerConfig_DATATYPE_ID 9
#define SilKit_FlexrayClusterParameters_DATATYPE_ID 10
#define SilKit_FlexrayNodeParameters_DATATYPE_ID 11
#define SilKit_FlexrayHostCommand_DATATYPE_ID 12
#define SilKit_FlexrayHeader_DATATYPE_ID 13
#define SilKit_FlexrayFrame_DATATYPE_ID 14
#define SilKit_FlexrayTxBufferConfig_DATATYPE_ID 15
#define SilKit_FlexrayTxBufferUpdate_DATATYPE_ID 16

// Flexray data type versions
#define SilKit_FlexrayFrameEvent_VERSION 1
#define SilKit_FlexrayFrameTransmitEvent_VERSION 1
#define SilKit_FlexraySymbolEvent_VERSION 1
#define SilKit_FlexraySymbolTransmitEvent_VERSION 1
#define SilKit_FlexrayCycleStartEvent_VERSION 1
#define SilKit_FlexrayPocStatusEvent_VERSION 1
#define SilKit_FlexrayWakeupEvent_VERSION 1
#define SilKit_FlexrayControllerConfig_VERSION 1
#define SilKit_FlexrayClusterParameters_VERSION 1
#define SilKit_FlexrayNodeParameters_VERSION 1
#define SilKit_FlexrayHostCommand_VERSION 1
#define SilKit_FlexrayHeader_VERSION 1
#define SilKit_FlexrayFrame_VERSION 1
#define SilKit_FlexrayTxBufferConfig_VERSION 1
#define SilKit_FlexrayTxBufferUpdate_VERSION 1

// Flexray make versioned IDs
#define SilKit_FlexrayFrameEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayFrameEvent)
#define SilKit_FlexrayFrameTransmitEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayFrameTransmitEvent)
#define SilKit_FlexraySymbolEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexraySymbolEvent)
#define SilKit_FlexraySymbolTransmitEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexraySymbolTransmitEvent)
#define SilKit_FlexrayCycleStartEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayCycleStartEvent)
#define SilKit_FlexrayPocStatusEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayPocStatusEvent)
#define SilKit_FlexrayWakeupEvent_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayWakeupEvent)
#define SilKit_FlexrayControllerConfig_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayControllerConfig)
#define SilKit_FlexrayClusterParameters_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayClusterParameters)
#define SilKit_FlexrayNodeParameters_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayNodeParameters)
#define SilKit_FlexrayHostCommand_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayHostCommand)
#define SilKit_FlexrayHeader_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayHeader)
#define SilKit_FlexrayFrame_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayFrame)
#define SilKit_FlexrayTxBufferConfig_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayTxBufferConfig)
#define SilKit_FlexrayTxBufferUpdate_STRUCT_VERSION SK_ID_MAKE(Flexray, SilKit_FlexrayTxBufferUpdate)

// LIN

// LIN data type IDs
#define SilKit_LinFrame_DATATYPE_ID 1
#define SilKit_LinFrameResponse_DATATYPE_ID 2
#define SilKit_LinControllerConfig_DATATYPE_ID 3
#define SilKit_LinFrameStatusEvent_DATATYPE_ID 4
#define SilKit_LinGoToSleepEvent_DATATYPE_ID 5
#define SilKit_LinWakeupEvent_DATATYPE_ID 6
#define SilKit_Experimental_LinSlaveConfigurationEvent_DATATYPE_ID 7
#define SilKit_Experimental_LinSlaveConfiguration_DATATYPE_ID 8
#define SilKit_Experimental_LinControllerDynamicConfig_DATATYPE_ID 9
#define SilKit_Experimental_LinFrameHeaderEvent_DATATYPE_ID 10
#define SilKit_LinSendFrameHeaderRequest_DATATYPE_ID 11

// LIN data type versions
#define SilKit_LinFrame_VERSION 1
#define SilKit_LinFrameResponse_VERSION 1
#define SilKit_LinControllerConfig_VERSION 1
#define SilKit_LinFrameStatusEvent_VERSION 1
#define SilKit_LinGoToSleepEvent_VERSION 1
#define SilKit_LinWakeupEvent_VERSION 1
#define SilKit_Experimental_LinSlaveConfigurationEvent_VERSION 1
#define SilKit_Experimental_LinSlaveConfiguration_VERSION 1
#define SilKit_Experimental_LinControllerDynamicConfig_VERSION 1
#define SilKit_Experimental_LinFrameHeaderEvent_VERSION 1
#define SilKit_LinSendFrameHeaderRequest_VERSION 1

// LIN make versioned IDs
#define SilKit_LinFrame_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinFrame)
#define SilKit_LinFrameResponse_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinFrameResponse)
#define SilKit_LinControllerConfig_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinControllerConfig)
#define SilKit_LinFrameStatusEvent_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinFrameStatusEvent)
#define SilKit_LinGoToSleepEvent_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinGoToSleepEvent)
#define SilKit_LinWakeupEvent_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinWakeupEvent)
#define SilKit_Experimental_LinSlaveConfigurationEvent_STRUCT_VERSION \
    SK_ID_MAKE(Lin, SilKit_Experimental_LinSlaveConfigurationEvent)
#define SilKit_Experimental_LinSlaveConfiguration_STRUCT_VERSION \
    SK_ID_MAKE(Lin, SilKit_Experimental_LinSlaveConfiguration)
#define SilKit_Experimental_LinControllerDynamicConfig_STRUCT_VERSION \
    SK_ID_MAKE(Lin, SilKit_Experimental_LinControllerDynamicConfig)
#define SilKit_Experimental_LinFrameHeaderEvent_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_Experimental_LinFrameHeaderEvent)
#define SilKit_LinSendFrameHeaderRequest_STRUCT_VERSION SK_ID_MAKE(Lin, SilKit_LinSendFrameHeaderRequest)

// Data
// Data data type IDs
#define SilKit_DataMessageEvent_DATATYPE_ID 1
#define SilKit_DataSpec_DATATYPE_ID 2

// Data data type versions
#define SilKit_DataMessageEvent_VERSION 1
#define SilKit_DataSpec_VERSION 1

// Data public API IDs
#define SilKit_DataMessageEvent_STRUCT_VERSION SK_ID_MAKE(Data, SilKit_DataMessageEvent)
#define SilKit_DataSpec_STRUCT_VERSION SK_ID_MAKE(Data, SilKit_DataSpec)

// Rpc
// Rpc data type IDs
#define SilKit_RpcCallEvent_DATATYPE_ID 1
#define SilKit_RpcCallResultEvent_DATATYPE_ID 2
#define SilKit_RpcSpec_DATATYPE_ID 3

// Rpc data type Versions
#define SilKit_RpcCallEvent_VERSION 1
#define SilKit_RpcCallResultEvent_VERSION 1
#define SilKit_RpcSpec_VERSION 1

// Rpc public API IDs
#define SilKit_RpcCallEvent_STRUCT_VERSION SK_ID_MAKE(Rpc, SilKit_RpcCallEvent)
#define SilKit_RpcCallResultEvent_STRUCT_VERSION SK_ID_MAKE(Rpc, SilKit_RpcCallResultEvent)
#define SilKit_RpcSpec_STRUCT_VERSION SK_ID_MAKE(Rpc, SilKit_RpcSpec)

// Participant
// Participant data type IDs
#define SilKit_ParticipantStatus_DATATYPE_ID 1
#define SilKit_LifecycleConfiguration_DATATYPE_ID 2
#define SilKit_WorkflowConfiguration_DATATYPE_ID 3
#define SilKit_ParticipantConnectionInformation_DATATYPE_ID 4

// Participant data type Versions
#define SilKit_ParticipantStatus_VERSION 1
#define SilKit_LifecycleConfiguration_VERSION 1
#define SilKit_WorkflowConfiguration_VERSION 3
#define SilKit_ParticipantConnectionInformation_VERSION 1

// Participant public API IDs
#define SilKit_ParticipantStatus_STRUCT_VERSION SK_ID_MAKE(Participant, SilKit_ParticipantStatus)
#define SilKit_LifecycleConfiguration_STRUCT_VERSION SK_ID_MAKE(Participant, SilKit_LifecycleConfiguration)
#define SilKit_WorkflowConfiguration_STRUCT_VERSION SK_ID_MAKE(Participant, SilKit_WorkflowConfiguration)
#define SilKit_ParticipantConnectionInformation_STRUCT_VERSION \
    SK_ID_MAKE(Participant, SilKit_ParticipantConnectionInformation)

// NetworkSimulator

// NetworkSimulator data type IDs
#define SilKit_Experimental_EventReceivers_DATATYPE_ID 1
#define SilKit_Experimental_SimulatedNetworkFunctions_DATATYPE_ID 2
#define SilKit_Experimental_SimulatedCanControllerFunctions_DATATYPE_ID 3
#define SilKit_Experimental_SimulatedFlexRayControllerFunctions_DATATYPE_ID 4
#define SilKit_Experimental_SimulatedEthernetControllerFunctions_DATATYPE_ID 5
#define SilKit_Experimental_SimulatedLinControllerFunctions_DATATYPE_ID 6
#define SilKit_Experimental_NetSim_CanConfigureBaudrate_DATATYPE_ID 7
#define SilKit_Experimental_NetSim_CanControllerMode_DATATYPE_ID 8
#define SilKit_Experimental_NetSim_CanFrameRequest_DATATYPE_ID 9
#define SilKit_Experimental_NetSim_FlexrayControllerConfig_DATATYPE_ID 10
#define SilKit_Experimental_NetSim_FlexrayHostCommand_DATATYPE_ID 11
#define SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate_DATATYPE_ID 12
#define SilKit_Experimental_NetSim_FlexrayTxBufferUpdate_DATATYPE_ID 13
#define SilKit_Experimental_NetSim_EthernetFrameRequest_DATATYPE_ID 14
#define SilKit_Experimental_NetSim_EthernetControllerMode_DATATYPE_ID 15
#define SilKit_Experimental_NetSim_LinFrameRequest_DATATYPE_ID 16
#define SilKit_Experimental_NetSim_LinFrameHeaderRequest_DATATYPE_ID 17
#define SilKit_Experimental_NetSim_LinWakeupPulse_DATATYPE_ID 18
#define SilKit_Experimental_NetSim_LinControllerConfig_DATATYPE_ID 19
#define SilKit_Experimental_NetSim_LinFrameResponseUpdate_DATATYPE_ID 20
#define SilKit_Experimental_NetSim_LinControllerStatusUpdate_DATATYPE_ID 21

// NetworkSimulator data type Versions
#define SilKit_Experimental_EventReceivers_VERSION 1
#define SilKit_Experimental_SimulatedNetworkFunctions_VERSION 1
#define SilKit_Experimental_SimulatedCanControllerFunctions_VERSION 1
#define SilKit_Experimental_SimulatedFlexRayControllerFunctions_VERSION 1
#define SilKit_Experimental_SimulatedEthernetControllerFunctions_VERSION 1
#define SilKit_Experimental_SimulatedLinControllerFunctions_VERSION 1
#define SilKit_Experimental_NetSim_CanConfigureBaudrate_VERSION 1
#define SilKit_Experimental_NetSim_CanControllerMode_VERSION 1
#define SilKit_Experimental_NetSim_CanFrameRequest_VERSION 1
#define SilKit_Experimental_NetSim_FlexrayControllerConfig_VERSION 1
#define SilKit_Experimental_NetSim_FlexrayHostCommand_VERSION 1
#define SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate_VERSION 1
#define SilKit_Experimental_NetSim_FlexrayTxBufferUpdate_VERSION 1
#define SilKit_Experimental_NetSim_EthernetFrameRequest_VERSION 1
#define SilKit_Experimental_NetSim_EthernetControllerMode_VERSION 1
#define SilKit_Experimental_NetSim_LinFrameRequest_VERSION 1
#define SilKit_Experimental_NetSim_LinFrameHeaderRequest_VERSION 1
#define SilKit_Experimental_NetSim_LinWakeupPulse_VERSION 1
#define SilKit_Experimental_NetSim_LinControllerConfig_VERSION 1
#define SilKit_Experimental_NetSim_LinFrameResponseUpdate_VERSION 1
#define SilKit_Experimental_NetSim_LinControllerStatusUpdate_VERSION 1

// NetworkSimulator public API IDs
#define SilKit_Experimental_EventReceivers_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_EventReceivers)
#define SilKit_Experimental_SimulatedNetworkFunctions_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_SimulatedNetworkFunctions)
#define SilKit_Experimental_SimulatedCanControllerFunctions_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_SimulatedCanControllerFunctions)
#define SilKit_Experimental_SimulatedFlexRayControllerFunctions_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_SimulatedFlexRayControllerFunctions)
#define SilKit_Experimental_SimulatedEthernetControllerFunctions_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_SimulatedEthernetControllerFunctions)
#define SilKit_Experimental_SimulatedLinControllerFunctions_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_SimulatedLinControllerFunctions)
#define SilKit_Experimental_NetSim_CanConfigureBaudrate_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_CanConfigureBaudrate)
#define SilKit_Experimental_NetSim_CanControllerMode_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_CanControllerMode)
#define SilKit_Experimental_NetSim_CanFrameRequest_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_CanFrameRequest)
#define SilKit_Experimental_NetSim_FlexrayControllerConfig_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_FlexrayControllerConfig)
#define SilKit_Experimental_NetSim_FlexrayHostCommand_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_FlexrayHostCommand)
#define SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate)
#define SilKit_Experimental_NetSim_FlexrayTxBufferUpdate_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_FlexrayTxBufferUpdate)
#define SilKit_Experimental_NetSim_EthernetFrameRequest_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_EthernetFrameRequest)
#define SilKit_Experimental_NetSim_EthernetControllerMode_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_EthernetControllerMode)
#define SilKit_Experimental_NetSim_LinFrameRequest_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinFrameRequest)
#define SilKit_Experimental_NetSim_LinFrameHeaderRequest_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinFrameHeaderRequest)
#define SilKit_Experimental_NetSim_LinWakeupPulse_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinWakeupPulse)
#define SilKit_Experimental_NetSim_LinControllerConfig_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinControllerConfig)
#define SilKit_Experimental_NetSim_LinFrameResponseUpdate_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinFrameResponseUpdate)
#define SilKit_Experimental_NetSim_LinControllerStatusUpdate_STRUCT_VERSION \
    SK_ID_MAKE(NetworkSimulation, SilKit_Experimental_NetSim_LinControllerStatusUpdate)

SILKIT_END_DECLS
