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
/*! \brief A property of an existing service has changed (e.g. connection count, simulation status). */
#define SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated \
    ((SilKit_Experimental_ServiceDiscoveryEvent_Type)3)

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

/*! \brief Describes a single discovered service, passed by value to a service discovery handler.
 *
 * All pointer members are borrowed and only valid for the duration of the handler invocation. Copy
 * any data that must outlive the call.
 *
 * Fields \p connectedParticipantName and \p connectedServiceName are populated only in
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated events that represent a
 * connection change (pub/sub match or RPC call). They identify the single peer whose connection
 * was added or removed, while \p numberOfConnections gives the new running total. For all other
 * event types and all other service kinds these fields are empty strings.
 *
 * Fields \p isSimulated and \p simulatingParticipantName are set on bus controllers (CAN, Ethernet,
 * FlexRay, LIN) when a network simulator claims ownership of the controller's network. Both a
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated with \p isSimulated = true
 * (simulator already active) and a subsequent
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated are possible.
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
    //! The primary, user-facing identifier of the service: the network name for bus controllers,
    //! the topic for pub/sub, and the function name for RPC. Suitable as a display / join key for
    //! visualization and tooling.
    const char* primaryIdentifier;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    const char* mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers.
    SilKit_LabelList labelList;
    //! Reserved for future system-level simulation detection. Currently always an empty string.
    const char* simulationName;
    //! Name of the peer participant; populated only in ServiceUpdated connection events.
    const char* connectedParticipantName;
    //! Name of the peer service; populated only in ServiceUpdated connection events.
    const char* connectedServiceName;
    //! Name of the participant simulating this controller's network; empty when not simulated.
    const char* simulatingParticipantName;
    //! Number of active matched connections. Reported on the receiving side only: DataSubscribers count
    //! matched publishers and RpcServers count matched clients. Always 0 for DataPublishers, RpcClients
    //! and bus controllers.
    uint32_t numberOfConnections;
    //! True when a network simulator owns this bus controller's network.
    SilKit_Bool isSimulated;
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
 * subsequently invoked for every user-facing service created, updated, or removed.
 * Infrastructure/internal services are not reported. Network link events are not reported directly;
 * instead, affected bus controllers receive a
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated with \p isSimulated set.
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
