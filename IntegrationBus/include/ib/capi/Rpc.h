/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"
#include "ib/capi/IbMacros.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

/*! \brief The ib_Rpc_ExchangeFormat provides meta information for argument and return data of Rpc.
*   Except for this descriptive purpose, it is used for matching Rpc clients and servers.
*/
typedef struct {
    ib_InterfaceIdentifier interfaceId;
    /*! \brief The media type of the data as specified by RFC2046
    * (e.g. "application/xml", "application/vnd.google.protobuf", ...)
    * A null pointer is considered an invalid value. Only valid RFC2046 values are considered valid.
    * The only exception to this rule is an empty string.
    * An empty mediaType string is interpreted as undefined and matches any mediaType.
    */
    const char* mediaType;
} ib_Rpc_ExchangeFormat;

/*! \brief A unique handle of a remote call. */
typedef struct ib_Rpc_CallHandle ib_Rpc_CallHandle;

/*! \brief represents a handle to a Rpc_Server instance */
typedef struct ib_Rpc_Server ib_Rpc_Server;
/*! \brief represents a handle to a Rpc_Client instance */
typedef struct ib_Rpc_Client ib_Rpc_Client;

/*! \brief Properties of a discovered Rpc server
* 
* \param functionName The name of the function provided the Rpc server.
* \param exchangeFormat The exchangeFormat of the Rpc server.
* \param labelList The labels of the Rpc server.
*/
typedef struct ib_Rpc_DiscoveryResult
{
    ib_InterfaceIdentifier interfaceId;
    const char* functionName;
    ib_Rpc_ExchangeFormat* exchangeFormat;
    ib_KeyValueList* labelList;
} ib_Rpc_DiscoveryResult;

/*! \brief A list of discovered Rpc servers
* \param numResults The number of results.
* \param results The array containing the results.
*/
typedef struct ib_Rpc_DiscoveryResultList
{
    size_t numResults;
    ib_Rpc_DiscoveryResult results[1];
} ib_Rpc_DiscoveryResultList;



/*! The available result codes for calls issued by a client.
*/
typedef uint32_t ib_Rpc_CallStatus;
#define ib_Rpc_CallStatus_SUCCESS               ((uint32_t) 0)
#define ib_Rpc_CallStatus_SERVER_NOT_REACHABLE  ((uint32_t) 1)
#define ib_Rpc_CallStatus_UNDEFINED_ERROR       ((uint32_t) 2)

/*! \brief A callback function that is called on a Rpc server triggered by client.
* \param context The user provided context pointer that was provided when this handler was registered.
* \param server The Rpc server that received the call.
* \param callHandle The handle of this call, used to submit results.
* \param argumentData The incoming argument data of the call.
*/
typedef void (*ib_Rpc_CallHandler_t)(void* context, ib_Rpc_Server* server, ib_Rpc_CallHandle* callHandle,
    const ib_ByteVector* argumentData);

/*! \brief A handler that is called on a Rpc client when a Rpc server submitted a result to an earlier call
* of this client.
* \param context The user's context pointer that was provided when this handler was registered.
* \param client The Rpc client that received the result/triggered the invocation.
* \param callHandle The call handle that uniquely identifies the call.
* \param callStatus The status of the Rpc invocation. Communicates whether the call failed or succeeded.
* \param returnData The data of the call result.
*/
typedef void (*ib_Rpc_ResultHandler_t)(void* context, ib_Rpc_Client* client, ib_Rpc_CallHandle* callHandle,
                                       ib_Rpc_CallStatus callStatus, const ib_ByteVector* returnData);

/*! \brief A handler that is called with the results of a discovery query.
* \param context The user's context pointer that was provided when this handler was registered.
* \param discoveryResults A list of properties of discovered Rpc servers.
*/
typedef void (*ib_Rpc_DiscoveryResultHandler_t)(void* context, const ib_Rpc_DiscoveryResultList* discoveryResults);

/*! \brief Create a Rpc server on a simulation participant with the provided properties.
* \param out Pointer to which the resulting Rpc server reference will be written.
* \param participant The simulation participant for which the Rpc server should be created.
* \param functionName The name by which Rpc clients identify and connect to this Rpc server.
* \param exchangeFormat A meta description of the data that will be processed and returned by this Rpc server.
* \param labels A list of key-value pairs of this Rpc server. The labels are relevant for matching Rpc clients and
* Rpc servers.
* \param context A user provided context pointer that is passed to the callHandler on call.
* \param callHandler A callback function that is triggered on invocation of the server functionality.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Server_Create(ib_Rpc_Server** out, ib_SimulationParticipant* participant,
                                                     const char* functionName, ib_Rpc_ExchangeFormat* exchangeFormat,
                                                     const ib_KeyValueList* labels, void* context,
                                                     ib_Rpc_CallHandler_t callHandler);


typedef ib_ReturnCode (*ib_Rpc_Server_Create_t)(ib_Rpc_Server** out, ib_SimulationParticipant* participant,
                                                const char* functionName, ib_Rpc_ExchangeFormat* exchangeFormat,
                                                const ib_KeyValueList* labels, void* context,
                                                ib_Rpc_CallHandler_t callHandler);

/*! \brief Create a Rpc client on a simulation participant with the provided properties.
* \param out Pointer to which the resulting Rpc client reference will be written.
* \param participant The simulation participant for which the Rpc client should be created.
* \param functionName The functionName by which the Rpc server is identified.
* \param exchangeFormat A meta description of the data that will be provided by this client.
* \param labels A list of key-value pairs of this Rpc client. The labels are relevant for matching Rpc clients and 
* Rpc servers.
* \param context A user provided context that is reobtained on call result in the resultHandler.
* \param resultHandler A callback that is called when a call result is received.
* 
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Client_Create(ib_Rpc_Client** out, ib_SimulationParticipant* participant,
                                                     const char* functionName, ib_Rpc_ExchangeFormat* exchangeFormat,
                                                     const ib_KeyValueList* labels, void* context,
                                                     ib_Rpc_ResultHandler_t resultHandler);


typedef ib_ReturnCode (*ib_Rpc_Client_Create_t)(ib_Rpc_Client** out, ib_SimulationParticipant* participant,
                                                const char* functionName, ib_Rpc_ExchangeFormat* exchangeFormat,
                                                const ib_KeyValueList* labels, void* context,
                                                ib_Rpc_ResultHandler_t resultHandler);

/*! \brief Detach a call to one or multiple corresponding Rpc servers
* \param self The Rpc Client that should trigger the remote procedure call.
* \param outHandle The handle by which future results of this call can be identified.
* \param argumentData The data that should be transmitted to the Rpc server for this call
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Client_Call(ib_Rpc_Client* self, ib_Rpc_CallHandle** outHandle,
    const ib_ByteVector* argumentData);

typedef ib_ReturnCode(*ib_Rpc_Client_Call_t)(ib_Rpc_Client* self, ib_Rpc_CallHandle** outHandle,
    const ib_ByteVector* argumentData);


/*! \brief Submit a result for an earlier obtained call handle to an Rpc client.
* \param self The Rpc server that should submit the result of the remote procedure call.
* \param callHandle The call handle that was obtained earlier through an ib_Rpc_ResultHandler_t.
* \param returnData The data that should be returned to the calling client.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_Server_SubmitResult(ib_Rpc_Server* self, ib_Rpc_CallHandle* callHandle,
    const ib_ByteVector* returnData);

typedef ib_ReturnCode(*ib_Rpc_Server_SubmitResult_t)(ib_Rpc_Server* self, ib_Rpc_CallHandle* callHandle,
    const ib_ByteVector* returnData);

/*! \brief Query for available Rpc servers and their properties. The results are provided in the resultsHandler.
* \param participant The simulation participant launching the query.
* \param functionName Only discover Rpc servers with this functionName. Leave empty for a wildcard.
* \param exchangeFormat Only discover Rpc servers with this exchangeFormat. Leave empty for a wildcard.
* \param labels Only discover Rpc servers containing these labels. Use NULL to not filter for labels.
* \param context A user provided context that is reobtained in the resultHandler.
*/
IntegrationBusAPI ib_ReturnCode ib_Rpc_DiscoverServers(ib_SimulationParticipant* participant, const char* functionName,
                                                       ib_Rpc_ExchangeFormat* exchangeFormat,
                                                       const ib_KeyValueList* labels, void* context,
                                                       ib_Rpc_DiscoveryResultHandler_t resultHandler);

typedef ib_ReturnCode(*ib_Rpc_DiscoverServers_t)(ib_SimulationParticipant* participant, const char* functionName,
                                                ib_Rpc_ExchangeFormat* exchangeFormat, const ib_KeyValueList* labels,
                                                void* context, ib_Rpc_DiscoveryResultHandler_t resultHandler);

IB_END_DECLS

#pragma pack(pop)
