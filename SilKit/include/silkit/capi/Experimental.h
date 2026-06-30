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
//  Allows a participant to passively observe the services (bus controllers,
//  publishers/subscribers, RPC clients/servers, ...) created by all other
//  participants in the simulation. This mirrors the internal service discovery
//  used throughout the SIL Kit and requires no configuration of the observed
//  participants.
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

/*! \brief The type of a discovered service. Matches SilKit::Core::ServiceType. */
typedef uint32_t SilKit_Experimental_ServiceType;
#define SilKit_Experimental_ServiceType_Undefined ((SilKit_Experimental_ServiceType)0)
#define SilKit_Experimental_ServiceType_Link ((SilKit_Experimental_ServiceType)1)
#define SilKit_Experimental_ServiceType_Controller ((SilKit_Experimental_ServiceType)2)
#define SilKit_Experimental_ServiceType_SimulatedController ((SilKit_Experimental_ServiceType)3)
#define SilKit_Experimental_ServiceType_InternalController ((SilKit_Experimental_ServiceType)4)

/*! \brief Handler invoked when a service is created or removed in the simulation.
 *
 * The \p serviceDescriptor handle and any string returned by the
 * SilKit_Experimental_ServiceDescriptor_... accessors are only valid for the
 * duration of the handler invocation. Copy the data if it must outlive the call.
 *
 * \param context The user context pointer passed to \ref SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler.
 * \param eventType Whether the service was created or removed.
 * \param serviceDescriptor Opaque handle describing the affected service.
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

/*! \brief Register a handler that is called for every service in the simulation.
 *
 * Upon registration the handler is immediately invoked once for every service that is already known, each reported as
 * \ref SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated. It is subsequently invoked for every service
 * created or removed.
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

/*! \brief Return the name of the participant providing the service.
 *
 * The returned string is owned by the service descriptor and is only valid for the duration of the handler invocation.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetParticipantName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outParticipantName);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDescriptor_GetParticipantName_t)(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outParticipantName);

/*! \brief Return the name of the service.
 *
 * The returned string is owned by the service descriptor and is only valid for the duration of the handler invocation.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetServiceName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outServiceName);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDescriptor_GetServiceName_t)(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outServiceName);

/*! \brief Return the network name the service is associated with.
 *
 * The returned string is owned by the service descriptor and is only valid for the duration of the handler invocation.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetNetworkName(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outNetworkName);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDescriptor_GetNetworkName_t)(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char** outNetworkName);

/*! \brief Return the type of the service.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetServiceType(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, SilKit_Experimental_ServiceType* outServiceType);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDescriptor_GetServiceType_t)(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, SilKit_Experimental_ServiceType* outServiceType);

/*! \brief Look up a value in the supplemental data of the service by key (e.g. "topic", "controllerType").
 *
 * If the key is present, \p outHasValue is set to \ref SilKit_True and \p outValue points to the value. If the key is
 * absent, \p outHasValue is set to \ref SilKit_False, \p outValue is set to NULL, and \ref SilKit_ReturnCode_SUCCESS is
 * still returned. The returned string is owned by the service descriptor and is only valid for the duration of the
 * handler invocation.
 *
 * \warning This function is not part of the stable API and ABI of the SIL Kit. It may be removed at any time without
 *          prior notice.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char* key, const char** outValue,
    SilKit_Bool* outHasValue);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ServiceDescriptor_GetSupplementalDataItem_t)(
    const SilKit_Experimental_ServiceDescriptor* serviceDescriptor, const char* key, const char** outValue,
    SilKit_Bool* outHasValue);

SILKIT_END_DECLS

#pragma pack(pop)
