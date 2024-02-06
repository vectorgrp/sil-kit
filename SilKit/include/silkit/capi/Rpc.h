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

/*! \brief A pubsub/rpc node spec containing all matching relevant information */
typedef struct SilKit_RpcSpec
{
    SilKit_StructHeader structHeader;
    const char* functionName;
    const char* mediaType;
    SilKit_LabelList labelList;
} SilKit_RpcSpec;

/*! \brief A unique handle of a remote call. */
typedef struct SilKit_RpcCallHandle SilKit_RpcCallHandle;

/*! \brief Represents a handle to a RPC server instance */
typedef struct SilKit_RpcServer SilKit_RpcServer;
/*! \brief Represents a handle to a RPC client instance */
typedef struct SilKit_RpcClient SilKit_RpcClient;


/*! \brief The status of a RpcCallResultEvent. Informs whether a call was successful.
*/
typedef uint32_t SilKit_RpcCallStatus;
#define SilKit_RpcCallStatus_Success             ((SilKit_RpcCallStatus)0) //!< Call was successful
#define SilKit_RpcCallStatus_ServerNotReachable  ((SilKit_RpcCallStatus)1) //!< No server matching the RpcSpec was found
#define SilKit_RpcCallStatus_UndefinedError      ((SilKit_RpcCallStatus)2) //!< An unidentified error occured
/*! \brief The Call lead to an internal RpcServer error.
 * This might happen if no CallHandler was specified for the RpcServer.
 */
#define SilKit_RpcCallStatus_InternalServerError ((SilKit_RpcCallStatus)3)
/*! \brief The Call did run into a timeout and was canceled.
 * This might happen if a corresponding server crashed, ran into an error or took too long to answer the call
 */
#define SilKit_RpcCallStatus_Timeout             ((SilKit_RpcCallStatus)4)

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
    //! The user context pointer as it was provided when the call was triggered
    void* userContext;
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
typedef void (SilKitFPTR *SilKit_RpcCallHandler_t)(void* context, SilKit_RpcServer* server, const SilKit_RpcCallEvent* event);

/*! \brief A handler that is called on a RPC client when a RPC server submitted a result to an earlier call
 *          of this client.
 * \param context The user's context pointer that was provided when this handler was registered.
 * \param client The RPC client that received the result/triggered the invocation.
 * \param event The event contains information about the results of an earlier call of this client.
 */
typedef void (SilKitFPTR *SilKit_RpcCallResultHandler_t)(void* context, SilKit_RpcClient* client, const SilKit_RpcCallResultEvent* event);

/*! \brief Create a RPC server on a simulation participant with the provided properties.
 * \param outServer Pointer to which the resulting RPC server reference will be written.
 * \param participant The simulation participant for which the RPC server should be created.
 * \param controllerName The name of this controller (UTF-8).
 * \param rpcSpec A struct containing all matching related information
 * \param callHandlerContext A user provided context pointer that is passed to the callHandler on call.
 * \param callHandler A callback function that is triggered on invocation of the server functionality.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcServer_Create(SilKit_RpcServer** outServer,
                                                               SilKit_Participant* participant,
                                                               const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                               void* callHandlerContext,
                                                               SilKit_RpcCallHandler_t callHandler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_RpcServer_Create_t)(SilKit_RpcServer** out,
                                                                 SilKit_Participant* participant,
                                                                 const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                                 void* callHandlerContext,
                                                                 SilKit_RpcCallHandler_t callHandler);

/*! \brief Submit a result for an earlier obtained call handle to an RPC client.
 * \param self The RPC server that should submit the result of the remote procedure call.
 * \param callHandle The call handle that was obtained earlier through an SilKit_RpcCallResultHandler_t.
 * \param returnData The data that should be returned to the calling client.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SubmitResult(SilKit_RpcServer* self,
                                                                     SilKit_RpcCallHandle* callHandle,
                                                                     const SilKit_ByteVector* returnData);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_RpcServer_SubmitResult_t)(SilKit_RpcServer* self,
                                                                       SilKit_RpcCallHandle* callHandle,
                                                                       const SilKit_ByteVector* returnData);

/*! \brief Overwrite the call handler of a RPC server.
 * \param self The RPC server of which the callback should be overwritten.
 * \param context A user provided context pointer that is passed to the handler on call.
 * \param handler A callback function that is triggered on invocation of the server functionality.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SetCallHandler(SilKit_RpcServer* self, void* context,
                                                             SilKit_RpcCallHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_RpcServer_SetCallHandler_t)(SilKit_RpcServer* self, void* context,
                                                        SilKit_RpcCallHandler_t handler);

/*! \brief Create a RPC client on a simulation participant with the provided properties.
 * \param outClient Pointer to which the resulting RPC client reference will be written.
 * \param participant The simulation participant for which the RPC client should be created.
 * \param controllerName The name of this controller (UTF-8).
 * \param rpcSpec The specification of function name, media type and labels.
 * \param resultHandlerContext A user provided context that is reobtained on call result in the resultHandler.
 * \param resultHandler A callback that is called when a call result is received.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Create(SilKit_RpcClient** outClient,
                                                               SilKit_Participant* participant,
                                                               const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                               void* resultHandlerContext,
                                                               SilKit_RpcCallResultHandler_t resultHandler);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_RpcClient_Create_t)(SilKit_RpcClient** out,
                                                                 SilKit_Participant* participant,
                                                                 const char* controllerName, SilKit_RpcSpec* rpcSpec,
                                                                 void* resultHandlerContext,
                                                                 SilKit_RpcCallResultHandler_t resultHandler);

/*! \brief Dispatch a call to one or multiple corresponding RPC servers
 * \param self The RPC client that should trigger the remote procedure call.
 * \param argumentData The data that should be transmitted to the RPC server for this call.
 * \param userContext A user provided context pointer that is passed to the result handler when a result is received.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Call(SilKit_RpcClient* self,
    const SilKit_ByteVector* argumentData, void* userContext);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_RpcClient_Call_t)(SilKit_RpcClient* self,
    const SilKit_ByteVector* argumentData, void* userContext);

/*! \brief Initiate a remote procedure call with a specified timeout.
*
*  In a synchronized execution, simulation time is used for the timeout, 
*  in a unsynchronized execution, system time is used for the timeout.
* 
* \param self The RPC client that should trigger the remote procedure call.
* \param argumentData A non-owning reference to an opaque block of raw data
* \param timeout A duration in nanoseconds after which the call runs into a timeout and the CallResultHandler is called with status timeout. 
*  After the timeout occurred, no further call result events will be triggered for this call. 
* \param userContext An optional user provided pointer that is
* reobtained when receiving the call result.
*/

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcClient_CallWithTimeout(SilKit_RpcClient* self,
    const SilKit_ByteVector* argumentData, SilKit_NanosecondsTime timeout, void* userContext);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_RpcClient_CallWithTimeout_t)(SilKit_RpcClient* self,
    const SilKit_ByteVector* argumentData, SilKit_NanosecondsTime timeout, void* userContext);

/*! \brief Overwrite the call result handler of this client
 * \param self The RPC client that should trigger the remote procedure call.
 * \param context A user provided context pointer that is passed to the handler on call.
 * \param handler A callback that is called when a call result is received.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_RpcClient_SetCallResultHandler(SilKit_RpcClient* self, void* context,
                                                                   SilKit_RpcCallResultHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_RpcClient_SetCallResultHandler_t)(SilKit_RpcClient* self, void* context,
                                                              SilKit_RpcCallResultHandler_t handler);


SILKIT_END_DECLS

#pragma pack(pop)
