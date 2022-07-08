// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <limits.h>
#include <stdint.h>
#include <memory.h> //memset
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

SILKIT_BEGIN_DECLS

/*
typedef struct SilKit_StructHeader
{
    uint64_t version; // version 
    uint64_t nextPointer; //!< For future expansions
} SilKit_StructHeader_t;

#define SK_STRUCT_HEADER_MAGIC "SK"
*/

// Backward source compatibility for <3.99.27: [[deprecated]]
//typedef SilKit_StructHeader_t SilKit_InterfaceIdentifier;
typedef uint64_t SilKit_InterfaceIdentifier;

//Service Ids
#define SK_ID_SERVICE_START 0 //!< sentinel
#define SK_ID_SERVICE_Can 1
#define SK_ID_SERVICE_Ethernet 2
#define SK_ID_SERVICE_Flexray 3
#define SK_ID_SERVICE_Lin 4
#define SK_ID_SERVICE_Data 5
#define SK_ID_SERVICE_Rpc 6
#define SK_ID_SERVICE_Participant 7
#define SK_ID_SERVICE_END 8 //!< sentinel

#define SK_ID_DATATYPE_INVALID 0
#define SK_ID_VERSION_INVALID 0


//!< The identifiers service id
#define SK_ID_GET_SERVICE(ID) \
    ((ID >> 40)  & 0xFF)
//!< The identifiers data type id
#define SK_ID_GET_DATATYPE(ID)\
    ((ID >> 32)  & 0xFF)
//!< The identifiers data type version
#define SK_ID_GET_VERSION(ID)\
    ((ID >> 24)  & 0xFF)

#define SK_ID_MAKE(SERVICE_NAME, DATATYPE_NAME) \
    (SilKit_InterfaceIdentifier)(\
          ((SilKit_InterfaceIdentifier)83 /*S*/ << 56)\
        | ((SilKit_InterfaceIdentifier)75 /*K*/ << 48)\
        | ((SilKit_InterfaceIdentifier)(SK_ID_SERVICE_ ## SERVICE_NAME  & 0xFF) << 40)\
        | ((SilKit_InterfaceIdentifier)(SK_ID_DATATYPE_ ## DATATYPE_NAME & 0xff) << 32)\
        | ((SilKit_InterfaceIdentifier)(SK_ID_VERSION_ ## DATATYPE_NAME & 0xff) << 24)\
        | 0x0)
   
#define SK_ID_IS_VALID(ID)\
    (\
        (SK_ID_GET_SERVICE(ID) > SK_ID_SERVICE_START)\
        && (SK_ID_GET_SERVICE(ID) < SK_ID_SERVICE_END)\
        && (SK_ID_GET_DATATYPE(ID) != SK_ID_DATATYPE_INVALID)\
        && (SK_ID_GET_VERSION(ID) != SK_ID_VERSION_INVALID)\
        && ((ID >> 56 & 0xff) == /*S*/ 83)\
        && ((ID >> 48 & 0xff) == /*K*/ 75)\
    )

//!< Helper to access the Interface ID member
#define SilKit_Struct_GetId(VALUE) ((VALUE).interfaceId)
//!< Initialize the struct VALUE with a valid InterfaceId for the given type DATATYPE 
#define SilKit_Struct_Init(DATATYPE, VALUE) SilKit_Struct_GetId(VALUE) = SilKit_InterfaceIdentifier_ ## DATATYPE
// CAN
// CAN data type IDs
#define SK_ID_DATATYPE_CanFrame 1
#define SK_ID_DATATYPE_CanFrameTransmitEvent 2
#define SK_ID_DATATYPE_CanFrameEvent 3
#define SK_ID_DATATYPE_CanStateChangeEvent 4
#define SK_ID_DATATYPE_CanErrorStateChangeEvent 5

// CAN data type versions
#define SK_ID_VERSION_CanFrame 1
#define SK_ID_VERSION_CanFrameTransmitEvent 1
#define SK_ID_VERSION_CanFrameEvent 1
#define SK_ID_VERSION_CanStateChangeEvent 1
#define SK_ID_VERSION_CanErrorStateChangeEvent 1

// CAN make versioned IDs
#define SilKit_InterfaceIdentifier_CanFrame                     SK_ID_MAKE(Can, CanFrame)
#define SilKit_InterfaceIdentifier_CanFrameTransmitEvent        SK_ID_MAKE(Can, CanFrameTransmitEvent)
#define SilKit_InterfaceIdentifier_CanFrameEvent                SK_ID_MAKE(Can, CanFrameEvent)
#define SilKit_InterfaceIdentifier_CanStateChangeEvent          SK_ID_MAKE(Can, CanStateChangeEvent)
#define SilKit_InterfaceIdentifier_CanErrorStateChangeEvent     SK_ID_MAKE(Can, CanErrorStateChangeEvent)

// Ethernet
// 
// Ethernet data type IDs
#define SK_ID_DATATYPE_EthernetFrameEvent 1
#define SK_ID_DATATYPE_EthernetFrameTransmitEvent 2
#define SK_ID_DATATYPE_EthernetStateChangeEvent 3
#define SK_ID_DATATYPE_EthernetBitrateChangeEvent 4
#define SK_ID_DATATYPE_EthernetFrame 5

// Ethernet data type versions
#define SK_ID_VERSION_EthernetFrameEvent 1
#define SK_ID_VERSION_EthernetFrameTransmitEvent 1
#define SK_ID_VERSION_EthernetStateChangeEvent 1
#define SK_ID_VERSION_EthernetBitrateChangeEvent 1
#define SK_ID_VERSION_EthernetFrame 1

#define SilKit_InterfaceIdentifier_EthernetFrameEvent           SK_ID_MAKE(Ethernet, EthernetFrameEvent)
#define SilKit_InterfaceIdentifier_EthernetFrameTransmitEvent   SK_ID_MAKE(Ethernet, EthernetFrameTransmitEvent)
#define SilKit_InterfaceIdentifier_EthernetStateChangeEvent     SK_ID_MAKE(Ethernet, EthernetStateChangeEvent)
#define SilKit_InterfaceIdentifier_EthernetBitrateChangeEvent   SK_ID_MAKE(Ethernet, EthernetBitrateChangeEvent)
#define SilKit_InterfaceIdentifier_EthernetFrame                SK_ID_MAKE(Ethernet, EthernetFrame)

// FlexRay
// FlexRay data type IDs
#define SK_ID_DATATYPE_FlexrayFrameEvent 1
#define SK_ID_DATATYPE_FlexrayFrameTransmitEvent 2
#define SK_ID_DATATYPE_FlexraySymbolEvent 3
#define SK_ID_DATATYPE_FlexraySymbolTransmitEvent 5
#define SK_ID_DATATYPE_FlexrayCycleStartEvent 6
#define SK_ID_DATATYPE_FlexrayPocStatusEvent 7
#define SK_ID_DATATYPE_FlexrayWakeupEvent 8
#define SK_ID_DATATYPE_FlexrayControllerConfig 9
#define SK_ID_DATATYPE_FlexrayClusterParameters 10
#define SK_ID_DATATYPE_FlexrayNodeParameters 11
#define SK_ID_DATATYPE_FlexrayHostCommand 12
#define SK_ID_DATATYPE_FlexrayHeader 13
#define SK_ID_DATATYPE_FlexrayFrame 14
#define SK_ID_DATATYPE_FlexrayTxBufferConfig 15
#define SK_ID_DATATYPE_FlexrayTxBufferUpdate 16

// Flexray data type versions
#define SK_ID_VERSION_FlexrayFrameEvent 1
#define SK_ID_VERSION_FlexrayFrameTransmitEvent 1
#define SK_ID_VERSION_FlexraySymbolEvent 1
#define SK_ID_VERSION_FlexraySymbolTransmitEvent 1
#define SK_ID_VERSION_FlexrayCycleStartEvent 1
#define SK_ID_VERSION_FlexrayPocStatusEvent 1
#define SK_ID_VERSION_FlexrayWakeupEvent 1
#define SK_ID_VERSION_FlexrayControllerConfig 1
#define SK_ID_VERSION_FlexrayClusterParameters 1
#define SK_ID_VERSION_FlexrayNodeParameters 1
#define SK_ID_VERSION_FlexrayHostCommand 1
#define SK_ID_VERSION_FlexrayHeader 1
#define SK_ID_VERSION_FlexrayFrame 1
#define SK_ID_VERSION_FlexrayTxBufferConfig 1
#define SK_ID_VERSION_FlexrayTxBufferUpdate 1

// Flexray make versioned IDs
#define SilKit_InterfaceIdentifier_FlexrayFrameEvent            SK_ID_MAKE(Flexray, FlexrayFrameEvent)
#define SilKit_InterfaceIdentifier_FlexrayFrameTransmitEvent    SK_ID_MAKE(Flexray, FlexrayFrameTransmitEvent)
#define SilKit_InterfaceIdentifier_FlexraySymbolEvent           SK_ID_MAKE(Flexray, FlexraySymbolEvent)
#define SilKit_InterfaceIdentifier_FlexraySymbolTransmitEvent   SK_ID_MAKE(Flexray, FlexraySymbolTransmitEvent)
#define SilKit_InterfaceIdentifier_FlexrayCycleStartEvent       SK_ID_MAKE(Flexray, FlexrayCycleStartEvent)
#define SilKit_InterfaceIdentifier_FlexrayPocStatusEvent        SK_ID_MAKE(Flexray, FlexrayPocStatusEvent)
#define SilKit_InterfaceIdentifier_FlexrayWakeupEvent           SK_ID_MAKE(Flexray, FlexrayWakeupEvent)
#define SilKit_InterfaceIdentifier_FlexrayControllerConfig      SK_ID_MAKE(Flexray, FlexrayControllerConfig)
#define SilKit_InterfaceIdentifier_FlexrayClusterParameters     SK_ID_MAKE(Flexray, FlexrayClusterParameters)
#define SilKit_InterfaceIdentifier_FlexrayNodeParameters        SK_ID_MAKE(Flexray, FlexrayNodeParameters)
#define SilKit_InterfaceIdentifier_FlexrayHostCommand           SK_ID_MAKE(Flexray, FlexrayHostCommand)
#define SilKit_InterfaceIdentifier_FlexrayHeader                SK_ID_MAKE(Flexray, FlexrayHeader)
#define SilKit_InterfaceIdentifier_FlexrayFrame                 SK_ID_MAKE(Flexray, FlexrayFrame)
#define SilKit_InterfaceIdentifier_FlexrayTxBufferConfig        SK_ID_MAKE(Flexray, FlexrayTxBufferConfig)
#define SilKit_InterfaceIdentifier_FlexrayTxBufferUpdate        SK_ID_MAKE(Flexray, FlexrayTxBufferUpdate)

// Lin

// Lin data type IDs
#define SK_ID_DATATYPE_LinFrame 1
#define SK_ID_DATATYPE_LinFrameResponse 2
#define SK_ID_DATATYPE_LinControllerConfig 3
#define SK_ID_DATATYPE_LinFrameStatusEvent 4
#define SK_ID_DATATYPE_LinGoToSleepEvent 5
#define SK_ID_DATATYPE_LinWakeupEvent 6

// Lin data type versions
#define SK_ID_VERSION_LinFrame 1
#define SK_ID_VERSION_LinFrameResponse 1
#define SK_ID_VERSION_LinControllerConfig 1
#define SK_ID_VERSION_LinFrameStatusEvent 1
#define SK_ID_VERSION_LinGoToSleepEvent 1
#define SK_ID_VERSION_LinWakeupEvent 1

// Lin make versioned IDs
#define SilKit_InterfaceIdentifier_LinFrame                     SK_ID_MAKE(Lin, LinFrame)
#define SilKit_InterfaceIdentifier_LinFrameResponse             SK_ID_MAKE(Lin, LinFrameResponse)
#define SilKit_InterfaceIdentifier_LinControllerConfig          SK_ID_MAKE(Lin, LinControllerConfig)
#define SilKit_InterfaceIdentifier_LinFrameStatusEvent          SK_ID_MAKE(Lin, LinFrameStatusEvent)
#define SilKit_InterfaceIdentifier_LinGoToSleepEvent            SK_ID_MAKE(Lin, LinGoToSleepEvent)
#define SilKit_InterfaceIdentifier_LinWakeupEvent               SK_ID_MAKE(Lin, LinWakeupEvent)

// Data
// Data data type IDs
#define SK_ID_DATATYPE_DataMessageEvent 1
#define SK_ID_DATATYPE_NewDataPublisherEvent 2

// Data data type versions
#define SK_ID_VERSION_DataMessageEvent 1
#define SK_ID_VERSION_NewDataPublisherEvent 1

// Data public API IDs
#define SilKit_InterfaceIdentifier_DataMessageEvent             SK_ID_MAKE(Data, DataMessageEvent)
#define SilKit_InterfaceIdentifier_NewDataPublisherEvent        SK_ID_MAKE(Data, NewDataPublisherEvent)

// Rpc
// Rpc data type IDs
#define SK_ID_DATATYPE_RpcDiscoveryResult 1
#define SK_ID_DATATYPE_RpcCallEvent 2
#define SK_ID_DATATYPE_RpcCallResultEvent 3
#define SK_ID_DATATYPE_RpcDiscoveryResultList 4

// Rpc data type Versions
#define SK_ID_VERSION_RpcDiscoveryResult 1
#define SK_ID_VERSION_RpcCallEvent 1
#define SK_ID_VERSION_RpcCallResultEvent 1
#define SK_ID_VERSION_RpcDiscoveryResultList 1

// Rpc public API IDs
#define SilKit_InterfaceIdentifier_RpcDiscoveryResult           SK_ID_MAKE(Rpc, RpcDiscoveryResult)
#define SilKit_InterfaceIdentifier_RpcCallEvent                 SK_ID_MAKE(Rpc, RpcCallEvent)
#define SilKit_InterfaceIdentifier_RpcCallResultEvent           SK_ID_MAKE(Rpc, RpcCallResultEvent)
#define SilKit_InterfaceIdentifier_RpcDiscoveryResultList       SK_ID_MAKE(Rpc, RpcDiscoveryResultList)

// Participant
// Participant data type IDs
#define SK_ID_DATATYPE_ParticipantStatus 1

// Participant data type Versions
#define SK_ID_VERSION_ParticipantStatus 1

// Participant public API IDs
#define SilKit_InterfaceIdentifier_ParticipantStatus            SK_ID_MAKE(Participant, ParticipantStatus)
SILKIT_END_DECLS
