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

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/rpc/all.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>


namespace {

SilKit::Services::Rpc::RpcCallResultHandler MakeRpcCallResultHandler(void* context, SilKit_RpcCallResultHandler_t handler)
{
    return [handler, context](SilKit::Services::Rpc::IRpcClient* cppClient, const SilKit::Services::Rpc::RpcCallResultEvent& event) {
        auto* cClient = reinterpret_cast<SilKit_RpcClient*>(cppClient);
        SilKit_RpcCallResultEvent cEvent;
        SilKit_Struct_Init(SilKit_RpcCallResultEvent, cEvent);
        cEvent.timestamp = event.timestamp.count();
        cEvent.userContext = event.userContext;
        cEvent.callStatus = (SilKit_RpcCallStatus)event.callStatus;
        cEvent.resultData = ToSilKitByteVector(event.resultData);
        handler(context, cClient, &cEvent);
    };
}

SilKit::Services::Rpc::RpcCallHandler MakeRpcCallHandler(void* context, SilKit_RpcCallHandler_t handler)
{
    return [handler, context](SilKit::Services::Rpc::IRpcServer* cppServer, const SilKit::Services::Rpc::RpcCallEvent& event) {
        auto* cServer = reinterpret_cast<SilKit_RpcServer*>(cppServer);
        SilKit_RpcCallEvent cEvent;
        SilKit_Struct_Init(SilKit_RpcCallEvent, cEvent);
        cEvent.timestamp = event.timestamp.count();
        cEvent.callHandle = reinterpret_cast<SilKit_RpcCallHandle*>(event.callHandle);
        cEvent.argumentData = ToSilKitByteVector(event.argumentData);
        handler(context, cServer, &cEvent);
    };
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_RpcServer_Create(SilKit_RpcServer** out, SilKit_Participant* participant, const char* controllerName,
    SilKit_RpcSpec* rpcSpec, void* context, SilKit_RpcCallHandler_t callHandler)
try
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(rpcSpec);
    ASSERT_VALID_HANDLER_PARAMETER(callHandler);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    SilKit::Services::Rpc::RpcSpec cppDataSpec;
    assign(cppDataSpec, rpcSpec);
    auto rcpServer = cppParticipant->CreateRpcServer(controllerName, cppDataSpec,
                                                     MakeRpcCallHandler(context, callHandler));

    *out = reinterpret_cast<SilKit_RpcServer*>(rcpServer);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SubmitResult(SilKit_RpcServer* self, SilKit_RpcCallHandle* callHandle,
                                         const SilKit_ByteVector* returnData)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(callHandle);
    ASSERT_VALID_POINTER_PARAMETER(returnData);

    auto cppServer = reinterpret_cast<SilKit::Services::Rpc::IRpcServer*>(self);
    auto cppCallHandle = reinterpret_cast<SilKit::Services::Rpc::IRpcCallHandle*>(callHandle);
    cppServer->SubmitResult(cppCallHandle, SilKit::Util::ToSpan(*returnData));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_RpcServer_SetCallHandler(SilKit_RpcServer* self, void* context, SilKit_RpcCallHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(handler);

    auto cppServer = reinterpret_cast<SilKit::Services::Rpc::IRpcServer*>(self);
    cppServer->SetCallHandler(MakeRpcCallHandler(context, handler));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Create(SilKit_RpcClient** out, SilKit_Participant* participant, const char* controllerName,
                                   SilKit_RpcSpec* rpcSpec,
                                   void* context, SilKit_RpcCallResultHandler_t resultHandler)
try
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(rpcSpec);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);

    auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
    SilKit::Services::Rpc::RpcSpec cppRpcSpec;
    assign(cppRpcSpec, rpcSpec);
    auto rcpClient = cppParticipant->CreateRpcClient(controllerName, cppRpcSpec,
                                                     MakeRpcCallResultHandler(context, resultHandler));

    *out = reinterpret_cast<SilKit_RpcClient*>(rcpClient);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_RpcClient_Call(SilKit_RpcClient* self, const SilKit_ByteVector* argumentData, void* userContext)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(argumentData);

    auto cppClient = reinterpret_cast<SilKit::Services::Rpc::IRpcClient*>(self);
    cppClient->Call(SilKit::Util::ToSpan(*argumentData), userContext);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode SilKitCALL SilKit_RpcClient_CallWithTimeout(SilKit_RpcClient* self, const SilKit_ByteVector* argumentData,
    SilKit_NanosecondsTime duration, void* userContext)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(argumentData);

    auto cppClient = reinterpret_cast<SilKit::Services::Rpc::IRpcClient*>(self);
    cppClient->CallWithTimeout(SilKit::Util::ToSpan(*argumentData), std::chrono::nanoseconds{duration}, userContext);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_RpcClient_SetCallResultHandler(SilKit_RpcClient* self, void* context, SilKit_RpcCallResultHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(handler);

    auto cppClient = reinterpret_cast<SilKit::Services::Rpc::IRpcClient*>(self);
    cppClient->SetCallResultHandler(MakeRpcCallResultHandler(context, handler));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
