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
#define ib_InterfaceIdentifier_CanTransmitAcknowledge      ((ib_InterfaceIdentifier)1002001)
#define ib_InterfaceIdentifier_CanFrame_Meta               ((ib_InterfaceIdentifier)1003001)

// Ethernet
#define ib_InterfaceIdentifier_EthernetFrame               ((ib_InterfaceIdentifier)2001001)
#define ib_InterfaceIdentifier_EthernetTransmitAcknowledge ((ib_InterfaceIdentifier)2002001)
#define ib_InterfaceIdentifier_EthernetFrame_Meta          ((ib_InterfaceIdentifier)2003001)

// FlexRay
#define ib_InterfaceIdentifier_FlexRayMessage              ((ib_InterfaceIdentifier)3001001)
#define ib_InterfaceIdentifier_FlexRayMessageAck           ((ib_InterfaceIdentifier)3002001)
#define ib_InterfaceIdentifier_FlexRaySymbol               ((ib_InterfaceIdentifier)3003001)
#define ib_InterfaceIdentifier_FlexRaySymbolAck            ((ib_InterfaceIdentifier)3004001)
#define ib_InterfaceIdentifier_FlexRayCycleStart           ((ib_InterfaceIdentifier)3005001)
#define ib_InterfaceIdentifier_FlexRayControllerStatus     ((ib_InterfaceIdentifier)3006001)
#define ib_InterfaceIdentifier_FlexRayPocStatus            ((ib_InterfaceIdentifier)3007001)

// Lin
#define ib_InterfaceIdentifier_LinFrame                    ((ib_InterfaceIdentifier)4001001)
#define ib_InterfaceIdentifier_LinFrameResponse            ((ib_InterfaceIdentifier)4002001)
#define ib_InterfaceIdentifier_LinControllerConfig         ((ib_InterfaceIdentifier)4003001)

// Data
#define ib_InterfaceIdentifier_DataExchangeFormat          ((ib_InterfaceIdentifier)5001001)

// Rpc
#define ib_InterfaceIdentifier_RpcExchangeFormat           ((ib_InterfaceIdentifier)6001001)

IB_END_DECLS
