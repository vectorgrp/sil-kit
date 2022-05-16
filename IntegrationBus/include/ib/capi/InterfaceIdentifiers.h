// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <limits.h>
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"

IB_BEGIN_DECLS

typedef int32_t ib_InterfaceIdentifier;

// CAN
#define ib_InterfaceIdentifier_CanFrame                    ((ib_InterfaceIdentifier)1001001)
#define ib_InterfaceIdentifier_CanFrameTransmitEvent       ((ib_InterfaceIdentifier)1002001)
#define ib_InterfaceIdentifier_CanFrameEvent               ((ib_InterfaceIdentifier)1003001)
#define ib_InterfaceIdentifier_CanStateChangeEvent         ((ib_InterfaceIdentifier)1004001)
#define ib_InterfaceIdentifier_CanErrorStateChangeEvent    ((ib_InterfaceIdentifier)1005001)

// Ethernet
#define ib_InterfaceIdentifier_EthernetFrameEvent          ((ib_InterfaceIdentifier)2001001)
#define ib_InterfaceIdentifier_EthernetFrameTransmitEvent  ((ib_InterfaceIdentifier)2002001)
#define ib_InterfaceIdentifier_EthernetStateChangeEvent    ((ib_InterfaceIdentifier)2003001)
#define ib_InterfaceIdentifier_EthernetBitrateChangeEvent  ((ib_InterfaceIdentifier)2004001)

// FlexRay
#define ib_InterfaceIdentifier_FlexrayFrameEvent           ((ib_InterfaceIdentifier)3001001)
#define ib_InterfaceIdentifier_FlexrayFrameTransmitEvent   ((ib_InterfaceIdentifier)3002001)
#define ib_InterfaceIdentifier_FlexraySymbolEvent          ((ib_InterfaceIdentifier)3003001)
#define ib_InterfaceIdentifier_FlexraySymbolTransmitEvent  ((ib_InterfaceIdentifier)3004001)
#define ib_InterfaceIdentifier_FlexrayCycleStartEvent      ((ib_InterfaceIdentifier)3005001)
#define ib_InterfaceIdentifier_FlexrayPocStatusEvent       ((ib_InterfaceIdentifier)3006001)
#define ib_InterfaceIdentifier_FlexrayWakeupEvent          ((ib_InterfaceIdentifier)3007001)

// Lin
#define ib_InterfaceIdentifier_LinFrame                    ((ib_InterfaceIdentifier)4001001)
#define ib_InterfaceIdentifier_LinFrameResponse            ((ib_InterfaceIdentifier)4002001)
#define ib_InterfaceIdentifier_LinControllerConfig         ((ib_InterfaceIdentifier)4003001)
#define ib_InterfaceIdentifier_LinFrameStatusEvent         ((ib_InterfaceIdentifier)4004001)
#define ib_InterfaceIdentifier_LinGoToSleepEvent           ((ib_InterfaceIdentifier)4005001)
#define ib_InterfaceIdentifier_LinWakeupEvent              ((ib_InterfaceIdentifier)4006001)

// Data
#define ib_InterfaceIdentifier_DataMessageEvent            ((ib_InterfaceIdentifier)5001001)
#define ib_InterfaceIdentifier_NewDataPublisherEvent       ((ib_InterfaceIdentifier)5001002)

// Rpc
#define ib_InterfaceIdentifier_RpcDiscoveryResult          ((ib_InterfaceIdentifier)6001001)
#define ib_InterfaceIdentifier_RpcCallEvent                ((ib_InterfaceIdentifier)6002001)
#define ib_InterfaceIdentifier_RpcCallResultEvent          ((ib_InterfaceIdentifier)6003001)

// Participant
#define ib_InterfaceIdentifier_ParticipantStatus           ((ib_InterfaceIdentifier)7001001)

IB_END_DECLS
