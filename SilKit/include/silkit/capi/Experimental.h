// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
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
//  controllers, publishers/subscribers, RPC clients/servers) created by all
//  other participants in the simulation. This builds on the internal service
//  discovery used throughout the SIL Kit and requires no configuration of the
//  observed participants.
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
/*! \brief A link between two services: a pub/sub or RPC match, or a network-simulator link. */
#define SilKit_Experimental_ServiceKind_Link ((SilKit_Experimental_ServiceKind)9)

/*! \brief Describes a single discovered service, passed by value to a service discovery handler.
 *
 * All pointer members are borrowed and only valid for the duration of the handler invocation. Copy
 * any data that must outlive the call.
 *
 * A \ref SilKit_Experimental_ServiceKind_Link describes a link between two services: a pub/sub or
 * RPC match, or a network-simulator link. For a pub/sub or RPC match \p participantName /
 * \p serviceName name the receiving side (the DataSubscriber or RpcServer) and
 * \p connectedParticipantName / \p connectedServiceName name the peer (the DataPublisher or
 * RpcClient); \p primaryIdentifier is the topic or function name. For a network-simulator link
 * \p participantName is the simulating participant, \p primaryIdentifier is the simulated network
 * name (which matches the \p primaryIdentifier of the affected bus controllers), and the
 * \p connected... fields are empty.
 *
 * Links are reported as \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated.
 * Network-simulator links are additionally reported as
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved when the simulator detaches.
 * A pub/sub or RPC match link does not emit a removal event: its teardown always coincides with a
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved of one of its endpoints, from
 * which the disappearance of the link can be inferred.
 *
 * The \p connectedParticipantName and \p connectedServiceName fields are empty strings for all
 * kinds other than pub/sub and RPC \p SilKit_Experimental_ServiceKind_Link events.
 */
typedef struct
{
    SilKit_StructHeader structHeader;
    //! Name of the participant providing the service. For a Link this is the receiving side
    //! (subscriber/server) or, for a network-simulator link, the simulating participant.
    const char* participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    const char* serviceName;
    //! The kind of service.
    SilKit_Experimental_ServiceKind serviceKind;
    //! The primary, user-facing identifier of the service: the network name for bus controllers and
    //! network-simulator links, the topic for pub/sub, and the function name for RPC. Suitable as a
    //! display / join key for visualization and tooling.
    const char* primaryIdentifier;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    const char* mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers.
    SilKit_LabelList labelList;
    //! Reserved for future system-level simulation detection. Currently always an empty string.
    const char* simulationName;
    //! Name of the peer participant; populated only in pub/sub and RPC Link events.
    const char* connectedParticipantName;
    //! Name of the peer service; populated only in pub/sub and RPC Link events.
    const char* connectedServiceName;
} SilKit_Experimental_ServiceDescriptor;

/*! \brief Handler invoked when a user-facing service is created, updated, or removed in the simulation.
 *
 * The \p serviceDescriptor and all of its pointer members are only valid for the duration of the
 * handler invocation. Copy the data if it must outlive the call.
 *
 * \note Threading: this handler may be invoked on an internal SIL Kit worker thread or on an
 *       application thread that creates or destroys a service; the invoking thread is unspecified.
 *       Invocations are serialized (the handler is never called concurrently with itself). The
 *       handler must not block and must not call back into the participant that owns the observer,
 *       as doing so may deadlock.
 *
 * \param context The user context pointer passed to \ref SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler.
 * \param eventType Whether the service was created, updated, or removed.
 * \param serviceDescriptor The affected service.
 */
typedef void(SilKitFPTR* SilKit_Experimental_ServiceDiscoveryHandler_t)(
    void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor);

/*! \brief Obtain the experimental service discovery observer of a participant.
 *
 * The returned object is owned by the participant and must not be destroyed by the caller; there is
 * no corresponding destroy function. It refers to the participant's single service discovery, so
 * repeated calls for the same participant yield the same observer handle.
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
 * Upon registration the handler is immediately invoked once for every service that is already known,
 * each reported as \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated. It is
 * subsequently invoked for every user-facing service created or removed. Infrastructure/internal
 * services are not reported. Pub/sub and RPC matches as well as network-simulator links are reported
 * as \ref SilKit_Experimental_ServiceKind_Link services (see \ref SilKit_Experimental_ServiceDescriptor).
 *
 * \note Each call registers an additional, independent handler; handlers cannot be removed and remain
 *       registered for the lifetime of the participant. To observe with a single handler, call this
 *       function once. See the handler typedef for the threading contract.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 *
 * \param serviceDiscovery The observer obtained via \ref SilKit_Experimental_ServiceDiscovery_Create.
 * \param context The user context pointer made available to the handler.
 * \param handler The handler to be called on service creation, update, and removal.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler_t)(
    SilKit_Experimental_ServiceDiscovery* serviceDiscovery, void* context,
    SilKit_Experimental_ServiceDiscoveryHandler_t handler);

SILKIT_END_DECLS

#pragma pack(pop)
