// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <limits.h>
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"

SILKIT_BEGIN_DECLS

typedef int32_t SilKit_InterfaceIdentifier;

// CAN
#define SilKit_InterfaceIdentifier_CanFrame                    ((SilKit_InterfaceIdentifier)1001001)
#define SilKit_InterfaceIdentifier_CanFrameTransmitEvent       ((SilKit_InterfaceIdentifier)1002001)
#define SilKit_InterfaceIdentifier_CanFrameEvent               ((SilKit_InterfaceIdentifier)1003001)
#define SilKit_InterfaceIdentifier_CanStateChangeEvent         ((SilKit_InterfaceIdentifier)1004001)
#define SilKit_InterfaceIdentifier_CanErrorStateChangeEvent    ((SilKit_InterfaceIdentifier)1005001)

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
