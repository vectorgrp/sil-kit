// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

// ============================================================================
//  Experimental service discovery
//
//  Allows a participant to passively observe the user-facing services (bus
//  controllers, publishers/subscribers, RPC clients/servers, network links)
//  created by all other participants in the simulation. This builds on the
//  internal service discovery used throughout the SIL Kit and requires no
//  configuration of the observed participants.
//
//  \warning The functions and types declared in this header are not part of the
//           stable API and ABI of the SIL Kit. They may be removed or changed at
//           any time without prior notice.
// ============================================================================

/*! \brief The kind of change reported for a discovered service. */
typedef uint32_t SilKit_Experimental_ServiceDiscoveryEvent_Type;
/*! \brief An invalid / unknown service discovery event. */
#define SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid ((SilKit_Experimental_ServiceDiscoveryEvent_Type)0)
/*! \brief A service has been created (or was already present on registration). */
#define SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated \
    ((SilKit_Experimental_ServiceDiscoveryEvent_Type)1)
/*! \brief A service has been removed. */
#define SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved \
    ((SilKit_Experimental_ServiceDiscoveryEvent_Type)2)

/*! \brief The kind of a discovered service. Only user-facing services are reported. */
typedef uint32_t SilKit_Experimental_ServiceKind;
#define SilKit_Experimental_ServiceKind_Undefined ((SilKit_Experimental_ServiceKind)0)
#define SilKit_Experimental_ServiceKind_CanController ((SilKit_Experimental_ServiceKind)1)
#define SilKit_Experimental_ServiceKind_EthernetController ((SilKit_Experimental_ServiceKind)2)
#define SilKit_Experimental_ServiceKind_FlexrayController ((SilKit_Experimental_ServiceKind)3)
#define SilKit_Experimental_ServiceKind_LinController ((SilKit_Experimental_ServiceKind)4)
#define SilKit_Experimental_ServiceKind_DataPublisher ((SilKit_Experimental_ServiceKind)5)
#define SilKit_Experimental_ServiceKind_DataSubscriber ((SilKit_Experimental_ServiceKind)6)
#define SilKit_Experimental_ServiceKind_RpcClient ((SilKit_Experimental_ServiceKind)7)
#define SilKit_Experimental_ServiceKind_RpcServer ((SilKit_Experimental_ServiceKind)8)
#define SilKit_Experimental_ServiceKind_NetworkLink ((SilKit_Experimental_ServiceKind)9)

/*! \brief Describes a single discovered service, passed by value to a service discovery handler.
 *
 * \warning All pointer members (the strings and the label list) are borrowed and only valid for the duration of the
 *          handler invocation. Copy any data that must outlive the call.
 */
typedef struct
{
    SilKit_StructHeader structHeader;
    //! Name of the participant providing the service.
    const char* participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    const char* serviceName;
    //! The kind of service.
    SilKit_Experimental_ServiceKind serviceKind;
    //! Raw network/link identifier. The user-facing network name for bus controllers (e.g. "CAN1"); a generated id or
    //! "default" for pub/sub and RPC.
    const char* networkName;
    //! Convenience join key for visualization: the network name for bus controllers and links, the topic for
    //! pub/sub, and the function name for RPC.
    const char* networkOrTopic;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    const char* mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers and links.
    SilKit_LabelList labelList;
} SilKit_Experimental_ServiceDescriptor;

/*! \brief Handler invoked when a user-facing service is created or removed in the simulation.
 *
 * The \p serviceDescriptor and all of its pointer members are only valid for the duration of the handler invocation.
 * Copy the data if it must outlive the call.
 *
 * \param context The user context pointer passed to \ref SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler.
 * \param eventType Whether the service was created or removed.
 * \param serviceDescriptor The affected service.
 */
typedef void(SilKitFPTR* SilKit_Experimental_ServiceDiscoveryHandler_t)(
    void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor);

/*! \brief Obtain the experimental service discovery observer of a participant.
 *
 * The returned object is owned by the participant and must not be destroyed by the caller.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * \param outServiceDiscovery Pointer through which the service discovery observer is returned (out parameter).
 * \param participant The participant instance for which the observer is obtained.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_Create(
    SilKit_Experimental_ServiceDiscovery** outServiceDiscovery, SilKit_Participant* participant);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDiscovery_Create_t)(
    SilKit_Experimental_ServiceDiscovery** outServiceDiscovery, SilKit_Participant* participant);

/*! \brief Register a handler that is called for every user-facing service in the simulation.
 *
 * Upon registration the handler is immediately invoked once for every service that is already known, each reported as
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated. It is subsequently invoked for every user-facing
 * service created or removed. Internal/infrastructure services are not reported.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * \param serviceDiscovery The observer obtained via \ref SilKit_Experimental_ServiceDiscovery_Create.
 * \param context The user context pointer made available to the handler.
 * \param handler The handler to be called on service creation and removal.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler_t)(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler);

SILKIT_END_DECLS

#pragma pack(pop)
