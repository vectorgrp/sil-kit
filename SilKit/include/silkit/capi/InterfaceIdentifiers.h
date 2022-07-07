// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <limits.h>
#include <stdint.h>
#include <memory.h> //memset
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

SILKIT_BEGIN_DECLS

typedef struct SilKit_StructHeader
{
    uint64_t version;
    union
    {
        uint64_t _nextPointer64;
        void* nextPointer; //!< For future expansions
    };
} SilKit_StructHeader_t;

#define SK_STRUCT_HEADER_MAGIC "SK"

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
        && (SK_ID_GET_DATATYPE(ID) != 0)\
        && (SK_ID_GET_VERSION(ID) != 0)\
        && ((ID >> 56 & 0xff) == /*S*/ 83)\
        && ((ID >> 48 & 0xff) == /*K*/ 75)\
    )


// CAN

#define SK_ID_DATATYPE_CanFrame 1
#define SK_ID_VERSION_CanFrame 1
#define SilKit_InterfaceIdentifier_CanFrame                    SK_ID_MAKE(Can, CanFrame)
#define SK_ID_DATATYPE_CanFrameTransmitEvent 2
#define SK_ID_VERSION_CanFrameTransmitEvent 1
#define SilKit_InterfaceIdentifier_CanFrameTransmitEvent       SK_ID_MAKE(Can, CanFrameTransmitEvent)
#define SK_ID_DATATYPE_CanFrameEvent 3
#define SK_ID_VERSION_CanFrameEvent 1
#define SilKit_InterfaceIdentifier_CanFrameEvent               SK_ID_MAKE(Can, CanFrameEvent)
#define SK_ID_DATATYPE_CanStateChangeEvent 4
#define SK_ID_VERSION_CanStateChangeEvent 1
#define SilKit_InterfaceIdentifier_CanStateChangeEvent         SK_ID_MAKE(Can, CanStateChangeEvent)
#define SK_ID_DATATYPE_CanErrorStateChangeEvent 5
#define SK_ID_VERSION_CanErrorStateChangeEvent 1
#define SilKit_InterfaceIdentifier_CanErrorStateChangeEvent    SK_ID_MAKE(Can, CanErrorStateChangeEvent)

// Ethernet
#define SilKit_InterfaceIdentifier_EthernetFrameEvent          ((SilKit_InterfaceIdentifier)2001001)
#define SilKit_InterfaceIdentifier_EthernetFrameTransmitEvent  ((SilKit_InterfaceIdentifier)2002001)
#define SilKit_InterfaceIdentifier_EthernetStateChangeEvent    ((SilKit_InterfaceIdentifier)2003001)
#define SilKit_InterfaceIdentifier_EthernetBitrateChangeEvent  ((SilKit_InterfaceIdentifier)2004001)
#define SilKit_InterfaceIdentifier_EthernetFrame               ((SilKit_InterfaceIdentifier)2005001)

// FlexRay
#define SilKit_InterfaceIdentifier_FlexrayFrameEvent           ((SilKit_InterfaceIdentifier)3001001)
#define SilKit_InterfaceIdentifier_FlexrayFrameTransmitEvent   ((SilKit_InterfaceIdentifier)3002001)
#define SilKit_InterfaceIdentifier_FlexraySymbolEvent          ((SilKit_InterfaceIdentifier)3003001)
#define SilKit_InterfaceIdentifier_FlexraySymbolTransmitEvent  ((SilKit_InterfaceIdentifier)3004001)
#define SilKit_InterfaceIdentifier_FlexrayCycleStartEvent      ((SilKit_InterfaceIdentifier)3005001)
#define SilKit_InterfaceIdentifier_FlexrayPocStatusEvent       ((SilKit_InterfaceIdentifier)3006001)
#define SilKit_InterfaceIdentifier_FlexrayWakeupEvent          ((SilKit_InterfaceIdentifier)3007001)
#define SilKit_InterfaceIdentifier_FlexrayControllerConfig     ((SilKit_InterfaceIdentifier)3008001)
#define SilKit_InterfaceIdentifier_FlexrayClusterParameters    ((SilKit_InterfaceIdentifier)3009001)
#define SilKit_InterfaceIdentifier_FlexrayNodeParameters       ((SilKit_InterfaceIdentifier)3010001)
#define SilKit_InterfaceIdentifier_FlexrayHostCommand          ((SilKit_InterfaceIdentifier)3011001)
#define SilKit_InterfaceIdentifier_FlexrayHeader               ((SilKit_InterfaceIdentifier)3012001)
#define SilKit_InterfaceIdentifier_FlexrayFrame                ((SilKit_InterfaceIdentifier)3013001)
#define SilKit_InterfaceIdentifier_FlexrayTxBufferConfig       ((SilKit_InterfaceIdentifier)3014001)
#define SilKit_InterfaceIdentifier_FlexrayTxBufferUpdate       ((SilKit_InterfaceIdentifier)3015001)

// Lin
#define SilKit_InterfaceIdentifier_LinFrame                    ((SilKit_InterfaceIdentifier)4001001)
#define SilKit_InterfaceIdentifier_LinFrameResponse            ((SilKit_InterfaceIdentifier)4002001)
#define SilKit_InterfaceIdentifier_LinControllerConfig         ((SilKit_InterfaceIdentifier)4003001)
#define SilKit_InterfaceIdentifier_LinFrameStatusEvent         ((SilKit_InterfaceIdentifier)4004001)
#define SilKit_InterfaceIdentifier_LinGoToSleepEvent           ((SilKit_InterfaceIdentifier)4005001)
#define SilKit_InterfaceIdentifier_LinWakeupEvent              ((SilKit_InterfaceIdentifier)4006001)

// Data
#define SilKit_InterfaceIdentifier_DataMessageEvent            ((SilKit_InterfaceIdentifier)5001001)
#define SilKit_InterfaceIdentifier_NewDataPublisherEvent       ((SilKit_InterfaceIdentifier)5001002)

// Rpc
#define SilKit_InterfaceIdentifier_RpcDiscoveryResult          ((SilKit_InterfaceIdentifier)6001001)
#define SilKit_InterfaceIdentifier_RpcCallEvent                ((SilKit_InterfaceIdentifier)6002001)
#define SilKit_InterfaceIdentifier_RpcCallResultEvent          ((SilKit_InterfaceIdentifier)6003001)
#define SilKit_InterfaceIdentifier_RpcDiscoveryResultList      ((SilKit_InterfaceIdentifier)6004001)

// Participant
#define SilKit_InterfaceIdentifier_ParticipantStatus           ((SilKit_InterfaceIdentifier)7001001)

SILKIT_END_DECLS
