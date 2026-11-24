// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stddef.h>
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/Rpc.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

// ============================================================================
//  Experimental management client
//
//  A registry with `Experimental.Management.Enable` hosts a management participant in the
//  simulation `.well-known/sil-kit/management`. The management client joins that
//  simulation and reports the simulations on the registry, their participants
//  and required participants, and the configurations participants shared via
//  `Experimental.Management.ShareConfiguration`.
//
//  Handlers are invoked on a SIL Kit thread, except that the Set...Handler
//  functions invoke a new handler immediately on the calling thread and
//  SilKit_Experimental_ManagementClient_Destroy invokes the handlers of pending
//  requests. Handlers must not block and must not destroy the client. All
//  pointers passed to a handler are only valid for the duration of the
//  invocation; copy what must outlive it.
//
//  \warning The functions and types declared in this header are not part of the
//           stable API and ABI of the SIL Kit. They may be removed or changed at
//           any time without prior notice.
// ============================================================================

/*! \brief Information about the registry, see \ref SilKit_Experimental_ManagementClient_SetRegistryInfoHandler. */
typedef struct
{
    SilKit_StructHeader structHeader;
    uint32_t apiVersionMajor;
    uint32_t apiVersionMinor;
    //! SIL Kit version of the registry.
    const char* silKitVersion;
    //! Features offered by the management participant, e.g. "simulations", "participant-configuration".
    const SilKit_StringList* features;
} SilKit_Experimental_ManagementRegistryInfo;

/*! \brief A simulation on the registry. */
typedef struct
{
    SilKit_StructHeader structHeader;
    const char* name;
    const SilKit_StringList* participantNames;
    //! As last set by a SystemController of the simulation; empty if none was set.
    const SilKit_StringList* requiredParticipantNames;
} SilKit_Experimental_ManagementSimulation;

/*! \brief All simulations on the registry and the change to the previous state, by simulation name. */
typedef struct
{
    SilKit_StructHeader structHeader;
    //! Incremented by the management participant on every change.
    uint64_t sequence;
    size_t numSimulations;
    const SilKit_Experimental_ManagementSimulation* simulations;
    const SilKit_StringList* createdSimulationNames;
    const SilKit_StringList* removedSimulationNames;
    //! Simulations whose participants or required participants changed.
    const SilKit_StringList* changedSimulationNames;
} SilKit_Experimental_ManagementSimulationsEvent;

typedef uint8_t SilKit_Experimental_ManagementParticipantConfigurationStatus;
#define SilKit_Experimental_ManagementParticipantConfigurationStatus_Available \
    ((SilKit_Experimental_ManagementParticipantConfigurationStatus)0)
//! The participant is connected, but did not set `Experimental.Management.ShareConfiguration`.
#define SilKit_Experimental_ManagementParticipantConfigurationStatus_NotShared \
    ((SilKit_Experimental_ManagementParticipantConfigurationStatus)1)
#define SilKit_Experimental_ManagementParticipantConfigurationStatus_UnknownParticipant \
    ((SilKit_Experimental_ManagementParticipantConfigurationStatus)2)

/*! \brief Reply to \ref SilKit_Experimental_ManagementClient_GetParticipantConfiguration. */
typedef struct
{
    SilKit_StructHeader structHeader;
    SilKit_Experimental_ManagementParticipantConfigurationStatus status;
    //! The participant configuration as JSON if \p status is Available, an empty string otherwise.
    const char* configurationJson;
} SilKit_Experimental_ManagementParticipantConfigurationReply;

typedef void(SilKitFPTR* SilKit_Experimental_ManagementRegistryInfoHandler_t)(
    void* context, SilKit_Experimental_ManagementClient* client,
    const SilKit_Experimental_ManagementRegistryInfo* registryInfo);

typedef void(SilKitFPTR* SilKit_Experimental_ManagementSimulationsHandler_t)(
    void* context, SilKit_Experimental_ManagementClient* client,
    const SilKit_Experimental_ManagementSimulationsEvent* simulationsEvent);

/*! \p reply is NULL unless \p callStatus is \ref SilKit_RpcCallStatus_Success. */
typedef void(SilKitFPTR* SilKit_Experimental_ManagementParticipantConfigurationHandler_t)(
    void* context, SilKit_Experimental_ManagementClient* client, SilKit_RpcCallStatus callStatus,
    const SilKit_Experimental_ManagementParticipantConfigurationReply* reply);

/*! \brief Create a management client for the registry at \p registryUri; any simulation path is ignored.
 *
 * The client joins the management simulation as a participant with a generated name. Destroy it with
 * \ref SilKit_Experimental_ManagementClient_Destroy.
 *
 * \param outClient Pointer through which the client is returned (out parameter).
 * \param participantConfiguration Configuration of the client participant; it is copied.
 * \param registryUri URI of the registry, e.g. "silkit://localhost:8500".
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_Create(
    SilKit_Experimental_ManagementClient** outClient, SilKit_ParticipantConfiguration* participantConfiguration,
    const char* registryUri);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ManagementClient_Create_t)(
    SilKit_Experimental_ManagementClient** outClient, SilKit_ParticipantConfiguration* participantConfiguration,
    const char* registryUri);

/*! \brief Destroy a management client.
 *
 * Handlers of pending requests are invoked with \ref SilKit_RpcCallStatus_UndefinedError before this function
 * returns; they must not use the client. No handler is invoked after this function returned.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_Experimental_ManagementClient_Destroy(SilKit_Experimental_ManagementClient* client);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ManagementClient_Destroy_t)(
    SilKit_Experimental_ManagementClient* client);

/*! \brief Set the handler for the registry info, replacing a previous one.
 *
 * If the info is already known, the handler is invoked immediately. If the registry has no management participant
 * (older SIL Kit or `Experimental.Management.Enable` not set), it is never invoked.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_SetRegistryInfoHandler(
    SilKit_Experimental_ManagementClient* client, void* context,
    SilKit_Experimental_ManagementRegistryInfoHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ManagementClient_SetRegistryInfoHandler_t)(
    SilKit_Experimental_ManagementClient* client, void* context,
    SilKit_Experimental_ManagementRegistryInfoHandler_t handler);

/*! \brief Set the handler for changes of the simulations on the registry, replacing a previous one.
 *
 * If a state is already known, the handler is invoked immediately, reporting all simulations as created.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_Experimental_ManagementClient_SetSimulationsHandler(SilKit_Experimental_ManagementClient* client, void* context,
                                                           SilKit_Experimental_ManagementSimulationsHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ManagementClient_SetSimulationsHandler_t)(
    SilKit_Experimental_ManagementClient* client, void* context,
    SilKit_Experimental_ManagementSimulationsHandler_t handler);

/*! \brief Request the configuration a participant shared with the registry.
 *
 * \p handler is invoked exactly once, with a call status other than \ref SilKit_RpcCallStatus_Success if the request
 * failed, e.g. on timeout or if the management participant is not reachable.
 *
 * \param client The management client.
 * \param simulationName Name of the simulation the participant joined.
 * \param participantName Name of the participant.
 * \param timeout Timeout of the request in nanoseconds.
 * \param context User context passed to \p handler.
 * \param handler Invoked with the result of the request.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_GetParticipantConfiguration(
    SilKit_Experimental_ManagementClient* client, const char* simulationName, const char* participantName,
    SilKit_NanosecondsTime timeout, void* context,
    SilKit_Experimental_ManagementParticipantConfigurationHandler_t handler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_ManagementClient_GetParticipantConfiguration_t)(
    SilKit_Experimental_ManagementClient* client, const char* simulationName, const char* participantName,
    SilKit_NanosecondsTime timeout, void* context,
    SilKit_Experimental_ManagementParticipantConfigurationHandler_t handler);

SILKIT_END_DECLS

#pragma pack(pop)
