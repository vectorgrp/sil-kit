// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/NetworkSimulator.h"

#include "silkit/capi/Can.h"
#include "silkit/capi/Flexray.h"
#include "silkit/capi/Ethernet.h"
#include "silkit/capi/Lin.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

typedef struct SilKit_Experimental_EventProducer SilKit_Experimental_EventProducer;
typedef struct SilKit_Experimental_CanEventProducer SilKit_Experimental_CanEventProducer;
typedef struct SilKit_Experimental_FlexRayEventProducer SilKit_Experimental_FlexRayEventProducer;
typedef struct SilKit_Experimental_EthernetEventProducer SilKit_Experimental_EthernetEventProducer;
typedef struct SilKit_Experimental_LinEventProducer SilKit_Experimental_LinEventProducer;

typedef uint64_t SilKit_Experimental_ControllerDescriptor;
#define SilKit_Experimental_ControllerDescriptor_Invalid ((SilKit_Experimental_ControllerDescriptor)0)

struct SilKit_Experimental_EventReceivers
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    size_t numReceivers;
    const SilKit_Experimental_ControllerDescriptor* controllerDescriptors;
};
typedef struct SilKit_Experimental_EventReceivers SilKit_Experimental_EventReceivers;

// --------------------------------
// CAN
// --------------------------------

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_CanEventProducer_Produce(SilKit_Experimental_CanEventProducer* eventProducer,
                                                                       SilKit_StructHeader* msg,
                                                                       const SilKit_Experimental_EventReceivers* receivers);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_CanEventProducer_Produce_t)(SilKit_Experimental_CanEventProducer* eventProducer,
                                                                         SilKit_StructHeader* msg,
                                                                         const SilKit_Experimental_EventReceivers* receivers);

// --------------------------------
// FlexRay
// --------------------------------

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_FlexRayEventProducer_Produce(SilKit_Experimental_FlexRayEventProducer* eventProducer,
                                                                           SilKit_StructHeader* msg,
                                                                           const SilKit_Experimental_EventReceivers* receivers);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_FlexRayEventProducer_Produce_t)(SilKit_Experimental_FlexRayEventProducer* eventProducer,
                                                                             SilKit_StructHeader* msg,
                                                                             const SilKit_Experimental_EventReceivers* receivers);

// --------------------------------
// Ethernet
// --------------------------------

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_EthernetEventProducer_Produce(SilKit_Experimental_EthernetEventProducer* eventProducer,
                                                                            SilKit_StructHeader* msg,
                                                                            const SilKit_Experimental_EventReceivers* receivers);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_EthernetEventProducer_Produce_t)(
    SilKit_Experimental_EthernetEventProducer* eventProducer, SilKit_StructHeader* msg, const SilKit_Experimental_EventReceivers* receivers);


// --------------------------------
// Lin
// --------------------------------

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinEventProducer_Produce(SilKit_Experimental_LinEventProducer* eventProducer,
                                                                       SilKit_StructHeader* msg,
                                                                       const SilKit_Experimental_EventReceivers* receivers);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_LinEventProducer_Produce_t)(SilKit_Experimental_LinEventProducer* eventProducer,
                                                                         SilKit_StructHeader* msg,
                                                                         const SilKit_Experimental_EventReceivers* receivers);

SILKIT_END_DECLS

#pragma pack(pop)
