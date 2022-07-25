/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */


#pragma once
#include <stdint.h>
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/SilKitMacros.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

/*! \brief A unique handle of a remote call. */
typedef struct SilKit_RpcCallHandle SilKit_RpcCallHandle;

/*! \brief represents a handle to a RPC server instance */
typedef struct SilKit_RpcServer SilKit_RpcServer;
/*! \brief represents a handle to a RPC client instance */
typedef struct SilKit_RpcClient SilKit_RpcClient;

/*! \brief Properties of a discovered RPC server
*
* \param functionName The name of the function provided the RPC server.
* \param mediaType The mediaType of the RPC server.
* \param labelList The labels of the RPC server.
*/
typedef struct SilKit_RpcDiscoveryResult
{
    SilKit_StructHeader structHeader;
    const char* functionName;
    const char* mediaType;
    SilKit_KeyValueList* labelList;
} SilKit_RpcDiscoveryResult;

/*! \brief A list of discovered RPC servers
* \param numResults The number of results.
* \param results The array containing the results.
*/
typedef struct SilKit_RpcDiscoveryResultList
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    size_t numResults;
    SilKit_RpcDiscoveryResult* results;
} SilKit_RpcDiscoveryResultList;

/*! The available result codes for calls issued by a client.
*/
typedef uint32_t SilKit_RpcCallStatus;
#define SilKit_CallStatus_SUCCESS               ((uint32_t) 0)
#define SilKit_CallStatus_SERVER_NOT_REACHABLE  ((uint32_t) 1)
#define SilKit_CallStatus_UNDEFINED_ERROR       ((uint32_t) 2)

typedef struct {
    SilKit_StructHeader structHeader;
    //! Send timestamp of the event
    SilKit_NanosecondsTime timestamp;
    //! The handle of this call, used to submit results
    SilKit_RpcCallHandle* callHandle;
    //! The incoming argument data of the call
    SilKit_ByteVector argumentData;
} SilKit_RpcCallEvent;

typedef struct {
    SilKit_StructHeader structHeader;
    //! Send timestamp of the event
    SilKit_NanosecondsTime timestamp;
    //! The call handle that uniquely identifies the call
    SilKit_RpcCallHandle* callHandle;
    //! The status of the RPC invocation. Communicates whether the call failed or succeeded
    SilKit_RpcCallStatus callStatus;
    //! The data of the call result
    SilKit_ByteVector resultData;
} SilKit_RpcCallResultEvent;

/*! \brief A callback function that is called on a RPC server triggered by client.
* \param context The user provided context pointer that was provided when this handler was registered.
* \param server The RPC server that received the call.
* \param event The event contains information about the call by the client.
*/
typedef void (*SilKit_RpcCallHandler_t)(void* context, SilKit_RpcServer* server, const SilKit_RpcCallEvent* event);

/*! \brief A handler that is called on a RPC client when a RPC server submitted a result to an earlier call
*          of this client.
* \param context The user's context pointer that was provided when this handler was registered.
* \param client The RPC client that received the result/triggered the invocation.
* \param event The event contains information about the results of an earlier call of this client.
*/
typedef void (*SilKit_RpcCallResultHandler_t)(void* context, SilKit_RpcClient* client, const SilKit_RpcCallResultEvent* event);

/*! \brief A handler that is called with the results of a discovery query.
* \param context The user's context pointer that was provided when this handler was registered.
* \param discoveryResults A list of properties of discovered RPC servers.
*/
typedef void (*SilKit_RpcDiscoveryResultHandler_t)(void* context, const SilKit_RpcDiscoveryResultList* discoveryResults);

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
SilKitAPI SilKit_ReturnCode SilKit_RpcServer_Create(SilKit_RpcServer** out, SilKit_Participant* participant,
                                                     const char* controllerName, const char* functionName,
                                                     const char* mediaType, const SilKit_KeyValueList* labels,
                                                     void* context, SilKit_RpcCallHandler_t callHandler);

typedef SilKit_ReturnCode (*SilKit_RpcServer_Create_t)(SilKit_RpcServer** out, SilKit_Participant* participant,
                                                const char* controllerName, const char* functionName,
                                                const char* mediaType, const SilKit_KeyValueList* labels, void* context,
                                                SilKit_RpcCallHandler_t callHandler);

/*! \brief Submit a result for an earlier obtained call handle to an RPC client.
* \param self The RPC server that should submit the result of the remote procedure call.
* \param callHandle The call handle that was obtained earlier through an SilKit_RpcCallResultHandler_t.
* \param returnData The data that should be returned to the calling client.
*/
SilKitAPI SilKit_ReturnCode SilKit_RpcServer_SubmitResult(SilKit_RpcServer* self, SilKit_RpcCallHandle* callHandle,
                                                           const SilKit_ByteVector* returnData);

typedef SilKit_ReturnCode (*SilKit_RpcServer_SubmitResult_t)(SilKit_RpcServer* self, SilKit_RpcCallHandle* callHandle,
                                                      const SilKit_ByteVector* returnData);

/*! \brief Overwrite the call handler of a RPC server.
* \param self The RPC server of which the callback should be overwritten.
* \param context A user provided context pointer that is passed to the handler on call.
* \param handler A callback function that is triggered on invocation of the server functionality.
*/
SilKitAPI SilKit_ReturnCode SilKit_RpcServer_SetCallHandler(SilKit_RpcServer* self, void* context,
                                                             SilKit_RpcCallHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_RpcServer_SetCallHandler_t)(SilKit_RpcServer* self, void* context,
                                                        SilKit_RpcCallHandler_t handler);

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
SilKitAPI SilKit_ReturnCode SilKit_RpcClient_Create(SilKit_RpcClient** out, SilKit_Participant* participant,
                                                     const char* controllerName, const char* functionName,
                                                     const char* mediaType, const SilKit_KeyValueList* labels,
                                                     void* context, SilKit_RpcCallResultHandler_t resultHandler);

typedef SilKit_ReturnCode (*SilKit_RpcClient_Create_t)(SilKit_RpcClient** out, SilKit_Participant* participant,
                                                const char* controllerName, const char* functionName,
                                                const char* mediaType, const SilKit_KeyValueList* labels, void* context,
                                                SilKit_RpcCallResultHandler_t resultHandler);

/*! \brief Dispatch a call to one or multiple corresponding RPC servers
* \param self The RPC client that should trigger the remote procedure call.
* \param outHandle The handle by which future results of this call can be identified.
* \param argumentData The data that should be transmitted to the RPC server for this call
*/
SilKitAPI SilKit_ReturnCode SilKit_RpcClient_Call(SilKit_RpcClient* self, SilKit_RpcCallHandle** outHandle,
    const SilKit_ByteVector* argumentData);

typedef SilKit_ReturnCode(*SilKit_RpcClient_Call_t)(SilKit_RpcClient* self, SilKit_RpcCallHandle** outHandle,
    const SilKit_ByteVector* argumentData);

/*! \brief Overwrite the call result handler of this client
* \param self The RPC client that should trigger the remote procedure call.
* \param context A user provided context pointer that is passed to the handler on call.
* \param handler A callback that is called when a call result is received.
*/
SilKitAPI SilKit_ReturnCode SilKit_RpcClient_SetCallResultHandler(SilKit_RpcClient* self, void* context,
                                                                   SilKit_RpcCallResultHandler_t handler);

typedef SilKit_ReturnCode (*SilKit_RpcClient_SetCallResultHandler_t)(SilKit_RpcClient* self, void* context,
                                                              SilKit_RpcCallResultHandler_t handler);

/*! \brief Query for available RPC servers and their properties. The results are provided in the resultsHandler.
* \param participant The simulation participant launching the query.
* \param functionName Only discover RPC servers with this functionName. Leave empty for a wildcard.
* \param mediaType Only discover RPC servers with this mediaType. Leave empty for a wildcard.
* \param labels Only discover RPC servers containing these labels. Use NULL to not filter for labels.
* \param context A user provided context that is reobtained in the resultHandler.
*/
SilKitAPI SilKit_ReturnCode SilKit_DiscoverServers(SilKit_Participant* participant, const char* functionName,
                                                       const char* mediaType, const SilKit_KeyValueList* labels,
                                                       void* context, SilKit_RpcDiscoveryResultHandler_t resultHandler);

typedef SilKit_ReturnCode (*SilKit_DiscoverServers_t)(SilKit_Participant* participant, const char* functionName,
                                                  const char* mediaType, const SilKit_KeyValueList* labels, void* context,
                                                  SilKit_RpcDiscoveryResultHandler_t resultHandler);

SILKIT_END_DECLS

#pragma pack(pop)
