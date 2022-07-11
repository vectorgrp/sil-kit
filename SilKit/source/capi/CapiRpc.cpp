// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
void assign(SilKit_RpcDiscoveryResultList** cResultList, const std::vector<SilKit::Services::Rpc::RpcDiscoveryResult>& cppDiscoveryResults)
{
    size_t numResults = cppDiscoveryResults.size();
    *cResultList = (SilKit_RpcDiscoveryResultList*)malloc(sizeof(SilKit_RpcDiscoveryResultList));
    if (*cResultList != NULL)
    {
        (*cResultList)->numResults = numResults;
        (*cResultList)->results = (SilKit_RpcDiscoveryResult*)malloc(numResults * sizeof(SilKit_RpcDiscoveryResult));
        if ((*cResultList)->results != NULL)
        {
            uint32_t i = 0;
            for (auto&& r : cppDiscoveryResults)
            {
                (*cResultList)->results[i].interfaceId = SilKit_InterfaceIdentifier_RpcDiscoveryResult;
                (*cResultList)->results[i].functionName = r.functionName.c_str();
        		(*cResultList)->results[i].mediaType = r.mediaType.c_str();
                assign(&(*cResultList)->results[i].labelList, r.labels);
                i++;
            };
        }
    }
}

auto GetDataOrNullptr(const std::vector<std::uint8_t>& vector) -> const std::uint8_t*
{
    if (vector.empty())
    {
        return nullptr;
    }
    return vector.data();
}

SilKit::Services::Rpc::RpcCallResultHandler MakeRpcCallResultHandler(void* context, SilKit_RpcCallResultHandler_t handler)
{
    return [handler, context](SilKit::Services::Rpc::IRpcClient* cppClient, const SilKit::Services::Rpc::RpcCallResultEvent& event) {
        auto* cClient = reinterpret_cast<SilKit_RpcClient*>(cppClient);
        SilKit_RpcCallResultEvent cEvent;
        cEvent.interfaceId = SilKit_InterfaceIdentifier_RpcCallResultEvent;
        cEvent.timestamp = event.timestamp.count();
        cEvent.callHandle = reinterpret_cast<SilKit_RpcCallHandle*>(event.callHandle);
        cEvent.callStatus = (SilKit_RpcCallStatus)event.callStatus;
        cEvent.resultData = SilKit_ByteVector{GetDataOrNullptr(event.resultData), event.resultData.size()};
        handler(context, cClient, &cEvent);
    };
}

SilKit::Services::Rpc::RpcCallHandler MakeRpcCallHandler(void* context, SilKit_RpcCallHandler_t handler)
{
    return [handler, context](SilKit::Services::Rpc::IRpcServer* cppServer, const SilKit::Services::Rpc::RpcCallEvent& event) {
        auto* cServer = reinterpret_cast<SilKit_RpcServer*>(cppServer);
        SilKit_RpcCallEvent cEvent;
        cEvent.interfaceId = SilKit_InterfaceIdentifier_RpcCallEvent;
        cEvent.timestamp = event.timestamp.count();
        cEvent.callHandle = reinterpret_cast<SilKit_RpcCallHandle*>(event.callHandle);
        cEvent.argumentData = SilKit_ByteVector{GetDataOrNullptr(event.argumentData), event.argumentData.size()};
        handler(context, cServer, &cEvent);
    };
}

}//namespace

extern "C" {

SilKit_ReturnCode SilKit_RpcServer_Create(SilKit_RpcServer** out, SilKit_Participant* participant, const char* controllerName,
                                   const char* functionName, const char* mediaType, const SilKit_KeyValueList* labels,
                                   void* context, SilKit_RpcCallHandler_t callHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(callHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpServer = cppParticipant->CreateRpcServer(controllerName, functionName, mediaType, cppLabels,
                                                         MakeRpcCallHandler(context, callHandler));

        *out = reinterpret_cast<SilKit_RpcServer*>(rcpServer);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_RpcServer_SubmitResult(SilKit_RpcServer* self, SilKit_RpcCallHandle* callHandle,
                                         const SilKit_ByteVector* returnData)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(callHandle);
    ASSERT_VALID_POINTER_PARAMETER(returnData);
    CAPI_ENTER
    {
        auto cppServer = reinterpret_cast<SilKit::Services::Rpc::IRpcServer*>(self);
        auto cppCallHandle = reinterpret_cast<SilKit::Services::Rpc::IRpcCallHandle*>(callHandle);
        auto cppReturnData =
            std::vector<uint8_t>(returnData->data, returnData->data+ returnData->size);
        cppServer->SubmitResult(cppCallHandle, cppReturnData);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_RpcServer_SetCallHandler(SilKit_RpcServer* self, void* context, SilKit_RpcCallHandler_t handler)
        {
            ASSERT_VALID_POINTER_PARAMETER(self);
            ASSERT_VALID_POINTER_PARAMETER(handler);
            CAPI_ENTER
            {
                auto cppServer = reinterpret_cast<SilKit::Services::Rpc::IRpcServer*>(self);
                cppServer->SetCallHandler(MakeRpcCallHandler(context, handler));
                return SilKit_ReturnCode_SUCCESS;
            }
            CAPI_LEAVE
        }

SilKit_ReturnCode SilKit_RpcClient_Create(SilKit_RpcClient** out, SilKit_Participant* participant, const char* controllerName,
                                   const char* functionName, const char* mediaType, const SilKit_KeyValueList* labels,
                                   void* context, SilKit_RpcCallResultHandler_t resultHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpClient = cppParticipant->CreateRpcClient(controllerName, functionName, mediaType, cppLabels,
                                                         MakeRpcCallResultHandler(context, resultHandler));

        *out = reinterpret_cast<SilKit_RpcClient*>(rcpClient);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_RpcClient_Call(SilKit_RpcClient* self, SilKit_RpcCallHandle** outHandle, const SilKit_ByteVector* argumentData)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_OUT_PARAMETER(outHandle);
    ASSERT_VALID_POINTER_PARAMETER(argumentData);
    CAPI_ENTER
    {
        auto cppClient = reinterpret_cast<SilKit::Services::Rpc::IRpcClient*>(self);
        auto cppCallHandle = cppClient->Call(
            std::vector<uint8_t>(argumentData->data, argumentData->data + argumentData->size));
        *outHandle = reinterpret_cast<SilKit_RpcCallHandle*>(cppCallHandle);
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_RpcClient_SetCallResultHandler(SilKit_RpcClient* self, void* context, SilKit_RpcCallResultHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppClient = reinterpret_cast<SilKit::Services::Rpc::IRpcClient*>(self);
        cppClient->SetCallResultHandler(MakeRpcCallResultHandler(context, handler));
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

SilKit_ReturnCode SilKit_DiscoverServers(SilKit_Participant* participant, const char* functionName, const char* mediaType,
                                     const SilKit_KeyValueList* labels, void* context,
                                     SilKit_RpcDiscoveryResultHandler_t resultHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(participant);
        auto cppMediaType = std::string(mediaType);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppParticipant->DiscoverRpcServers(
            functionName, cppMediaType, cppLabels,
            [resultHandler, context](const std::vector<SilKit::Services::Rpc::RpcDiscoveryResult>& cppDiscoveryResults) {
                SilKit_RpcDiscoveryResultList* results;
                assign(&results, cppDiscoveryResults);
                resultHandler(context, results);
            });
        return SilKit_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} // extern "C"