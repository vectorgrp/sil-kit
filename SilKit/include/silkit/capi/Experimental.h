// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/NetworkSimulator.h"
#include "silkit/capi/Orchestration.h"

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
/*! \brief A service has been created (or was already present on registration, see \p isSnapshot). */
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
/*! \brief A network simulator simulates a network. \p participantName is the simulating participant,
 *         \p primaryIdentifier the simulated network name and \p networkType its bus type; both match
 *         the \p primaryIdentifier and \p networkType of the affected bus controllers. */
#define SilKit_Experimental_ServiceKind_NetworkSimulatorLink ((SilKit_Experimental_ServiceKind)9)
/*! \brief A DataPublisher is matched with a DataSubscriber. \p participantName / \p serviceName name
 *         the DataSubscriber, \p connectedParticipantName / \p connectedServiceName the DataPublisher,
 *         and \p primaryIdentifier is the topic. */
#define SilKit_Experimental_ServiceKind_PubSubMatch ((SilKit_Experimental_ServiceKind)10)
/*! \brief An RpcClient is matched with an RpcServer. \p participantName / \p serviceName name the
 *         RpcServer, \p connectedParticipantName / \p connectedServiceName the RpcClient, and
 *         \p primaryIdentifier is the function name. */
#define SilKit_Experimental_ServiceKind_RpcMatch ((SilKit_Experimental_ServiceKind)11)
/*! \brief The lifecycle of a participant, reported once the participant starts its lifecycle.
 *         \p operationMode is its operation mode. */
#define SilKit_Experimental_ServiceKind_LifecycleService ((SilKit_Experimental_ServiceKind)12)
/*! \brief The time synchronization of a participant, reported once the participant starts its
 *         lifecycle. \p timeSyncActive tells whether the participant takes part in the distributed
 *         virtual time synchronization. */
#define SilKit_Experimental_ServiceKind_TimeSyncService ((SilKit_Experimental_ServiceKind)13)

/*! \brief Describes a single discovered service, passed by value to a service discovery handler.
 *
 * All pointer members are borrowed and only valid for the duration of the handler invocation. Copy
 * any data that must outlive the call. Fields that do not apply to a kind are empty strings, zero,
 * \ref SilKit_NetworkType_Undefined or \ref SilKit_OperationMode_Invalid.
 *
 * A service is identified by \p participantName and \p serviceId; both are reported unchanged on
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated and
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved.
 *
 * Network-simulator links and pub/sub and RPC matches have kinds of their own:
 * \ref SilKit_Experimental_ServiceKind_NetworkSimulatorLink,
 * \ref SilKit_Experimental_ServiceKind_PubSubMatch and \ref SilKit_Experimental_ServiceKind_RpcMatch.
 * Like all services, they are reported when they are established and when they are gone. A pub/sub or
 * RPC match is reported only while both of its endpoints are known: it is created after both endpoints,
 * and removed before the first of them. Its \p participantName / \p serviceName / \p serviceId name
 * the receiving side (DataSubscriber / RpcServer), its \p connected... fields the peer (DataPublisher /
 * RpcClient).
 */
typedef struct
{
    SilKit_StructHeader structHeader;
    //! Name of the participant providing the service. For a pub/sub or RPC match the receiving side
    //! (subscriber/server), for a network-simulator link the simulating participant.
    const char* participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    const char* serviceName;
    //! Identifier of the service, unique within its participant.
    uint64_t serviceId;
    //! The kind of service.
    SilKit_Experimental_ServiceKind serviceKind;
    //! The primary, user-facing identifier of the service: the network name for bus controllers and
    //! network-simulator links, the topic for pub/sub, and the function name for RPC; empty for
    //! lifecycle and time sync services. Suitable as a display / join key for visualization and tooling.
    const char* primaryIdentifier;
    //! Bus type of bus controllers and network-simulator links; \ref SilKit_NetworkType_Undefined
    //! otherwise.
    SilKit_Experimental_SimulatedNetworkType networkType;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    const char* mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers.
    SilKit_LabelList labelList;
    //! Operation mode of a \ref SilKit_Experimental_ServiceKind_LifecycleService;
    //! \ref SilKit_OperationMode_Invalid otherwise.
    SilKit_OperationMode operationMode;
    //! For a \ref SilKit_Experimental_ServiceKind_TimeSyncService: whether the participant takes part in
    //! the virtual time synchronization. SilKit_False otherwise.
    SilKit_Bool timeSyncActive;
    //! Name of the peer participant (publisher/client); populated only for pub/sub and RPC matches.
    const char* connectedParticipantName;
    //! Name of the peer service (publisher/client); populated only for pub/sub and RPC matches.
    const char* connectedServiceName;
    //! Identifier of the peer service (publisher/client) within its participant; populated only for
    //! pub/sub and RPC matches.
    uint64_t connectedServiceId;
    //! SilKit_True if the service already existed when the handler was registered (see
    //! \ref SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler).
    SilKit_Bool isSnapshot;
} SilKit_Experimental_ServiceDescriptor;

/*! \brief Handler invoked when a user-facing service is created or removed in the simulation.
 *
 * The \p serviceDescriptor and all of its pointer members are only valid for the duration of the
 * handler invocation. Copy the data if it must outlive the call.
 *
 * \note Threading: this handler may be invoked on an internal SIL Kit worker thread or on an
 *       application thread that creates or destroys a service; the invoking thread is unspecified.
 *       Invocations are serialized (the handler is never called concurrently with itself). The
 *       handler must not block and must not call back into the participant that owns the observer.
 *       Registering a handler from within a handler fails with \ref SilKit_ReturnCode_WRONGSTATE.
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
 * each reported as \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated with
 * \p isSnapshot set. These snapshot invocations happen synchronously, before this function returns. The
 * handler is subsequently invoked for every user-facing service created or removed. Infrastructure/internal
 * services are not reported. Network-simulator links and pub/sub and RPC matches are reported as
 * services of their own kinds (see \ref SilKit_Experimental_ServiceDescriptor).
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
