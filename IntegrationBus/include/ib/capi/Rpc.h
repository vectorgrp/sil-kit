/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"
#include "ib/capi/IbMacros.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

/*! \brief A unique handle of a remote call. */
typedef struct ib_Rpc_CallHandle ib_Rpc_CallHandle;

/*! \brief represents a handle to a RPC server instance */
typedef struct ib_Rpc_Server ib_Rpc_Server;
/*! \brief represents a handle to a RPC client instance */
typedef struct ib_Rpc_Client ib_Rpc_Client;

/*! \brief Properties of a discovered RPC server
*
* \param functionName The name of the function provided the RPC server.
* \param mediaType The mediaType of the RPC server.
* \param labelList The labels of the RPC server.
*/
typedef struct ib_Rpc_DiscoveryResult
{
    ib_InterfaceIdentifier interfaceId;
    const char* functionName;
    const char* mediaType;
    ib_KeyValueList* labelList;
} ib_Rpc_DiscoveryResult;

/*! \brief A list of discovered RPC servers
* \param numResults The number of results.
* \param results The array containing the results.
*/
typedef struct ib_Rpc_DiscoveryResultList
{
    size_t numResults;
    ib_Rpc_DiscoveryResult* results;
} ib_Rpc_DiscoveryResultList;

/*! The available result codes for calls issued by a client.
*/
typedef uint32_t ib_Rpc_CallStatus;
#define ib_Rpc_CallStatus_SUCCESS               ((uint32_t) 0)
#define ib_Rpc_CallStatus_SERVER_NOT_REACHABLE  ((uint32_t) 1)
#define ib_Rpc_CallStatus_UNDEFINED_ERROR       ((uint32_t) 2)

typedef struct {
    ib_InterfaceIdentifier interfaceId;
    //! Send timestamp of the event
    ib_NanosecondsTime timestamp;
    //! The handle of this call, used to submit results
    ib_Rpc_CallHandle* callHandle;
    //! The incoming argument data of the call
    ib_ByteVector argumentData;
} ib_Rpc_CallEvent;

typedef struct {
    ib_InterfaceIdentifier interfaceId;
    //! Send timestamp of the event
    ib_NanosecondsTime timestamp;
    //! The call handle that uniquely identifies the call
    ib_Rpc_CallHandle* callHandle;
    //! The status of the RPC invocation. Communicates whether the call failed or succeeded
    ib_Rpc_CallStatus callStatus;
    //! The data of the call result
    ib_ByteVector resultData;
} ib_Rpc_CallResultEvent;

/*! \brief A callback function that is called on a RPC server triggered by client.
* \param context The user provided context pointer that was provided when this handler was registered.
* \param server The RPC server that received the call.
* \param event The event contains information about the call by the client.
*/
typedef void (*ib_Rpc_CallHandler_t)(void* context, ib_Rpc_Server* server, const ib_Rpc_CallEvent* event);

/*! \brief A handler that is called on a RPC client when a RPC server submitted a result to an earlier call
*          of this client.
* \param context The user's context pointer that was provided when this handler was registered.
* \param client The RPC client that received the result/triggered the invocation.
* \param event The event contains information about the results of an earlier call of this client.
*/
typedef void (*ib_Rpc_CallResultHandler_t)(void* context, ib_Rpc_Client* client, const ib_Rpc_CallResultEvent* event);

/*! \brief A handler that is called with the results of a discovery query.
* \param context The user's context pointer that was provided when this handler was registered.
* \param discoveryResults A list of properties of discovered RPC servers.
*/
typedef void (*ib_Rpc_DiscoveryResultHandler_t)(void* context, const ib_Rpc_DiscoveryResultList* discoveryResults);

/*! \brief Create a RPC server on a simulation participant with the provided properties.
* \param out Pointer to which the resulting RPC server reference will be written.
* \param participant The simulation participant for which the RPC server should be created.
* \param controllerName The name of this controller.
* \param functionName The name by which RPC clients identify and connect to this RPC server.
* \param mediaType A meta description of the data that will be processed and returned by this RPC server.
* \param labels A list of key-value pairs of this RPC server. The labels are relevant for matching RPC clients and
*               RPC servers.
* \param context A user provided context pointer that is passed to the callHandler on call.
* \param callHandler A callback function that is triggered on invocation of the server functionality.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Server_Create(ib_Rpc_Server** out, ib_Participant* participant,
                                                     const char* controllerName, const char* functionName,
                                                     const char* mediaType, const ib_KeyValueList* labels,
                                                     void* context, ib_Rpc_CallHandler_t callHandler);

typedef ib_ReturnCode (*ib_Rpc_Server_Create_t)(ib_Rpc_Server** out, ib_Participant* participant,
                                                const char* controllerName, const char* functionName,
                                                const char* mediaType, const ib_KeyValueList* labels, void* context,
                                                ib_Rpc_CallHandler_t callHandler);

/*! \brief Submit a result for an earlier obtained call handle to an RPC client.
* \param self The RPC server that should submit the result of the remote procedure call.
* \param callHandle The call handle that was obtained earlier through an ib_Rpc_CallResultHandler_t.
* \param returnData The data that should be returned to the calling client.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Server_SubmitResult(ib_Rpc_Server* self, ib_Rpc_CallHandle* callHandle,
                                                           const ib_ByteVector* returnData);

typedef ib_ReturnCode (*ib_Rpc_Server_SubmitResult_t)(ib_Rpc_Server* self, ib_Rpc_CallHandle* callHandle,
                                                      const ib_ByteVector* returnData);

/*! \brief Overwrite the call handler of a RPC server.
* \param self The RPC server of which the callback should be overwritten.
* \param context A user provided context pointer that is passed to the handler on call.
* \param handler A callback function that is triggered on invocation of the server functionality.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Server_SetCallHandler(ib_Rpc_Server* self, void* context,
                                                             ib_Rpc_CallHandler_t handler);

typedef ib_ReturnCode (*ib_Rpc_Server_SetCallHandler_t)(ib_Rpc_Server* self, void* context,
                                                        ib_Rpc_CallHandler_t handler);

/*! \brief Create a RPC client on a simulation participant with the provided properties.
* \param out Pointer to which the resulting RPC client reference will be written.
* \param participant The simulation participant for which the RPC client should be created.
* \param controllerName The name of this controller.
* \param functionName The functionName by which the RPC server is identified.
* \param mediaType A meta description of the data that will be provided by this client.
* \param labels A list of key-value pairs of this RPC client. The labels are relevant for matching RPC clients and
*               RPC servers.
* \param context A user provided context that is reobtained on call result in the resultHandler.
* \param resultHandler A callback that is called when a call result is received.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Client_Create(ib_Rpc_Client** out, ib_Participant* participant,
                                                     const char* controllerName, const char* functionName,
                                                     const char* mediaType, const ib_KeyValueList* labels,
                                                     void* context, ib_Rpc_CallResultHandler_t resultHandler);

typedef ib_ReturnCode (*ib_Rpc_Client_Create_t)(ib_Rpc_Client** out, ib_Participant* participant,
                                                const char* controllerName, const char* functionName,
                                                const char* mediaType, const ib_KeyValueList* labels, void* context,
                                                ib_Rpc_CallResultHandler_t resultHandler);

/*! \brief Dispatch a call to one or multiple corresponding RPC servers
* \param self The RPC client that should trigger the remote procedure call.
* \param outHandle The handle by which future results of this call can be identified.
* \param argumentData The data that should be transmitted to the RPC server for this call
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Client_Call(ib_Rpc_Client* self, ib_Rpc_CallHandle** outHandle,
    const ib_ByteVector* argumentData);

typedef ib_ReturnCode(*ib_Rpc_Client_Call_t)(ib_Rpc_Client* self, ib_Rpc_CallHandle** outHandle,
    const ib_ByteVector* argumentData);

/*! \brief Overwrite the call result handler of this client
* \param self The RPC client that should trigger the remote procedure call.
* \param context A user provided context pointer that is passed to the handler on call.
* \param handler A callback that is called when a call result is received.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Client_SetCallResultHandler(ib_Rpc_Client* self, void* context,
                                                                   ib_Rpc_CallResultHandler_t handler);

typedef ib_ReturnCode (*ib_Rpc_Client_SetCallResultHandler_t)(ib_Rpc_Client* self, void* context,
                                                              ib_Rpc_CallResultHandler_t handler);

/*! \brief Query for available RPC servers and their properties. The results are provided in the resultsHandler.
* \param participant The simulation participant launching the query.
* \param functionName Only discover RPC servers with this functionName. Leave empty for a wildcard.
* \param mediaType Only discover RPC servers with this mediaType. Leave empty for a wildcard.
* \param labels Only discover RPC servers containing these labels. Use NULL to not filter for labels.
* \param context A user provided context that is reobtained in the resultHandler.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_DiscoverServers(ib_Participant* participant, const char* functionName,
                                                       const char* mediaType, const ib_KeyValueList* labels,
                                                       void* context, ib_Rpc_DiscoveryResultHandler_t resultHandler);

typedef ib_ReturnCode (*ib_Rpc_DiscoverServers_t)(ib_Participant* participant, const char* functionName,
                                                  const char* mediaType, const ib_KeyValueList* labels, void* context,
                                                  ib_Rpc_DiscoveryResultHandler_t resultHandler);

IB_END_DECLS

#pragma pack(pop)
