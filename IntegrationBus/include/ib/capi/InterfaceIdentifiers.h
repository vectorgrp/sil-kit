// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <limits.h>
#include <stdint.h>
#include "ib/capi/Utils.h"

__IB_BEGIN_DECLS

typedef int32_t ib_InterfaceIdentifier;

// CAN
#define     ib_InterfaceIdentifier_CanFrame               ((int32_t)  1002001)
#define     ib_InterfaceIdentifier_CanTransmitAcknowledge ((int32_t)  1003001)
#define     ib_InterfaceIdentifier_CanFrame_Meta          ((int32_t)  1004001)

// Ethernet
#define     ib_InterfaceIdentifier_EthernetFrame               ((int32_t)  2002001)
#define     ib_InterfaceIdentifier_EthernetTransmitAcknowledge ((int32_t)  2003001)
#define     ib_InterfaceIdentifier_EthernetFrame_Meta          ((int32_t)  2004001)

// FlexRay
#define     ib_InterfaceIdentifier_FlexRayMessage              ((ib_InterfaceIdentifier)3001001)
#define     ib_InterfaceIdentifier_FlexRayMessageAck           ((ib_InterfaceIdentifier)3002001)
#define     ib_InterfaceIdentifier_FlexRaySymbol               ((ib_InterfaceIdentifier)3003001)
#define     ib_InterfaceIdentifier_FlexRaySymbolAck            ((ib_InterfaceIdentifier)3004001)
#define     ib_InterfaceIdentifier_FlexRayCycleStart           ((ib_InterfaceIdentifier)3005001)
#define     ib_InterfaceIdentifier_FlexRayControllerStatus     ((ib_InterfaceIdentifier)3006001)
#define     ib_InterfaceIdentifier_FlexRayPocStatus            ((ib_InterfaceIdentifier)3007001)
// Data
#define     ib_InterfaceIdentifier_DataExchangeFormat          ((int32_t)  4002001)

__IB_END_DECLS
