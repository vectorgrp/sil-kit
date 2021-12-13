/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Types.h"
#include "ib/capi/DataPubSub.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

__IB_BEGIN_DECLS

/*! \brief  */
typedef struct ib_RPCServer ib_RPCServer;
/*! \brief  */
typedef struct ib_RPCClient ib_RPCClient;

/*! The available result codes for RPC calls issued by a client.
*/
typedef uint32_t ib_RPC_CallStatus;
#define ib_RPC_CallStatus_SUCCESS               ((uint32_t) 0) //!< Identifier Extension
#define ib_RPC_CallStatus_TRANSMIT_FAILED       ((uint32_t) 1) //!< Remote Transmission Request
#define ib_RPC_CallStatus_SERVER_NOT_REACHABLE  ((uint32_t) 2) //!< Remote Transmission Request
#define ib_RPC_CallStatus_UNDEFINED_ERROR       ((uint32_t) 3) //!< Remote Transmission Request

/*! \brief A handle that identifies a unique RPC call triggered by a rpc client */
typedef struct ib_RPC_CallHandle ib_RPC_CallHandle;

/*! \brief A handle that identifies a unique RPC result through which a server can send results for a certain call */
typedef struct ib_RPC_ResultHandle ib_RPC_ResultHandle;


/*! \brief A callback function that is called when a RPC call is triggered on a RPC server
* \param context The user provided context pointer that was provided when this handler was registered
* \param server The RPC server that received the RPC call.
* \param resultHandle A handle through which results to this rpc call can be submitted.
* \param data The data of the RPC call.
*/
typedef void (*ib_RPC_CallHandler_t)(void* context, ib_RPCServer* server, ib_RPC_ResultHandle* resultHandle
    const ib_ByteVector* data);

/*! \brief A callback function that is called when a RPC server returned a result to an earlier call of this client.
* A RPC call might receive multiple results for a single call.
* \param callStatus The status of the RPC invocation. Communicates whether the call failed or succeeded.
* \param context The user provided context pointer that was provided when this handler was registered.
* \param client The RPC client that received the result/triggered the invocation.
* \param callHandle The call handle that uniquely identifies this RPC call invocation.
* \param data The data of the RPC call result. Null pointer if call failed.
*/
typedef void (*ib_RPC_ResultHandler_t)(void* context, ib_RPC_CallStatus callStatus, ib_RPCClient* client,
    ib_RPC_CallHandle* callHandle, const ib_ByteVector* data);

/*! \brief Create a RPC server on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting RPC server reference will be written.
* \param participant The simulation participant for which the RPC server should be created.
* \param name The name by which RPC clients identify and connect to this RPC server.
* \param dataExchangeFormat A meta description of the data that will be processed and returned by this RPC server.
* \param context A user provided context pointer that is passed to the callHandler on call.
* \param callHandler A callback function that is triggered on invocation of the server functionality.
*/
CIntegrationBusAPI ib_ReturnCode ib_RPC_Server_Create(ib_RPCServer** out, ib_SimulationParticipant* participant,
    const char* name, ib_DataExchangeFormat* dataExchangeFormat, void* context, ib_RPC_CallHandler_t callHandler);

typedef ib_ReturnCode(*ib_RPCServer_create_t)(ib_RPCServer** out, ib_SimulationParticipant* participant,
    const char* name, ib_DataExchangeFormat* dataExchangeFormat, void* context, ib_RPC_CallHandler_t callHandler);

/*! \brief Create a RPC client on the provided simulation participant with the provided properties.
* \param out Pointer to which the resulting RPC client reference will be written.
* \param participant The simulation participant for which the RPC client should be created.
* \param name The name by which the RPC server is identified.
* \param dataExchangeFormat A meta description of the data that will be provided by this client.
* \param context A user provided context, that is reobtained on call result in the resultHandler.
* \param dataHandler A callback that is called when a rpc call result is received.
* 
*/
CIntegrationBusAPI ib_ReturnCode ib_RPC_Client_Create(ib_RPCClient** out, ib_SimulationParticipant* participant,
    const char* name, ib_DataExchangeFormat* dataExchangeFormat, void* context, ib_RPC_ResultHandler_t resultHandler);

typedef ib_ReturnCode(*ib_RPCClient_create_t)(ib_RPCClient** out, ib_SimulationParticipant* participant,
    const char* name, ib_DataExchangeFormat* dataExchangeFormat);

/*! \brief Sumbit a Call to one or multiple corresponding RPC servers
* \param self The RPC Client that should trigger the remote procedure call.
* \param outHandle The handle by which future results of this call can be identified.
* \param data The data that should be transmitted to the RPC server for this call
*/
CIntegrationBusAPI ib_ReturnCode ib_RPC_Client_Call(ib_RPCClient* self, ib_RPC_CallHandle** outHandle,
    const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_RPC_Client_Call_t)(ib_RPCClient* self, ib_RPC_CallHandle** outHandle,
    const ib_ByteVector* data);


/*! \brief Submit a Result for an earlier obtained call handle to an RPC client
* Might be called several times for an earlier obtained ib_RPC_CallHandle
* \param self The RPC server that should submit the result of the remote procedure call.
* \param resultHandle The result handle that was earlier obtained through an ib_RPC_CallHandler.
* \param data The data that should be published.
*/
CIntegrationBusAPI ib_ReturnCode ib_RPC_Server_SubmitResult(ib_RPCServer* self, ib_RPC_ResultHandle* resultHandle,
    const ib_ByteVector* data);

typedef ib_ReturnCode(*ib_RPC_Server_SubmitResult_t)(ib_RPCServer* self, ib_RPC_CallHandle* callHandle,
    const ib_ByteVector* data);

/*! \brief Free an rpc result handle when all results have been submitted.
* After this call, no additional results can be submitted for this rpc call
* \param server The RPC server that received this result handle
* \param resultHandle The result handle that was earlier obtained through an ib_RPC_CallHandler.
*/
CIntegrationBusAPI ib_ReturnCode ib_RPC_Server_FreeResultHandle(ib_RPCServer* self, ib_RPC_ResultHandle* resultHandle);

typedef ib_ReturnCode(*ib_RPC_Server_FreeResultHandle_t)(ib_RPCServer* self, ib_RPC_ResultHandle* resultHandle);

__IB_END_DECLS

#pragma pack(pop)
