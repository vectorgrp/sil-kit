// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/rpc/all.hpp"

#include "CapiImpl.hpp"
#include "TypeConversion.hpp"

#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

namespace {
void assign(ib_Rpc_DiscoveryResultList** cResultList, const std::vector<ib::sim::rpc::RpcDiscoveryResult>& cppDiscoveryResults)
{
    size_t numResults = cppDiscoveryResults.size();
    *cResultList = (ib_Rpc_DiscoveryResultList*)malloc(sizeof(ib_Rpc_DiscoveryResultList));
    if (*cResultList != NULL)
    {
        (*cResultList)->numResults = numResults;
        (*cResultList)->results = (ib_Rpc_DiscoveryResult*)malloc(numResults * sizeof(ib_Rpc_DiscoveryResult));
        if ((*cResultList)->results != NULL)
        {
            uint32_t i = 0;
            for (auto&& r : cppDiscoveryResults)
            {
                (*cResultList)->results[i].interfaceId = ib_InterfaceIdentifier_RpcDiscoveryResult;
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

ib::sim::rpc::RpcCallResultHandler MakeRpcCallResultHandler(void* context, ib_Rpc_CallResultHandler_t handler)
{
    return [handler, context](ib::sim::rpc::IRpcClient* cppClient, const ib::sim::rpc::RpcCallResultEvent& event) {
        auto* cClient = reinterpret_cast<ib_Rpc_Client*>(cppClient);
        ib_Rpc_CallResultEvent cEvent;
        cEvent.interfaceId = ib_InterfaceIdentifier_RpcCallResultEvent;
        cEvent.timestamp = event.timestamp.count();
        cEvent.callHandle = reinterpret_cast<ib_Rpc_CallHandle*>(event.callHandle);
        cEvent.callStatus = (ib_Rpc_CallStatus)event.callStatus;
        cEvent.resultData = ib_ByteVector{GetDataOrNullptr(event.resultData), event.resultData.size()};
        handler(context, cClient, &cEvent);
    };
}

ib::sim::rpc::RpcCallHandler MakeRpcCallHandler(void* context, ib_Rpc_CallHandler_t handler)
{
    return [handler, context](ib::sim::rpc::IRpcServer* cppServer, const ib::sim::rpc::RpcCallEvent& event) {
        auto* cServer = reinterpret_cast<ib_Rpc_Server*>(cppServer);
        ib_Rpc_CallEvent cEvent;
        cEvent.interfaceId = ib_InterfaceIdentifier_RpcCallEvent;
        cEvent.timestamp = event.timestamp.count();
        cEvent.callHandle = reinterpret_cast<ib_Rpc_CallHandle*>(event.callHandle);
        cEvent.argumentData = ib_ByteVector{GetDataOrNullptr(event.argumentData), event.argumentData.size()};
        handler(context, cServer, &cEvent);
    };
}

}//namespace

extern "C" {

ib_ReturnCode ib_Rpc_Server_Create(ib_Rpc_Server** out, ib_Participant* participant, const char* controllerName,
                                   const char* functionName, const char* mediaType, const ib_KeyValueList* labels,
                                   void* context, ib_Rpc_CallHandler_t callHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(callHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpServer = cppParticipant->CreateRpcServer(controllerName, functionName, mediaType, cppLabels,
                                                         MakeRpcCallHandler(context, callHandler));

        *out = reinterpret_cast<ib_Rpc_Server*>(rcpServer);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_Server_SubmitResult(ib_Rpc_Server* self, ib_Rpc_CallHandle* callHandle,
                                         const ib_ByteVector* returnData)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(callHandle);
    ASSERT_VALID_POINTER_PARAMETER(returnData);
    CAPI_ENTER
    {
        auto cppServer = reinterpret_cast<ib::sim::rpc::IRpcServer*>(self);
        auto cppCallHandle = reinterpret_cast<ib::sim::rpc::IRpcCallHandle*>(callHandle);
        auto cppReturnData =
            std::vector<uint8_t>(returnData->data, returnData->data+ returnData->size);
        cppServer->SubmitResult(cppCallHandle, cppReturnData);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_Server_SetCallHandler(ib_Rpc_Server* self, void* context, ib_Rpc_CallHandler_t handler)
        {
            ASSERT_VALID_POINTER_PARAMETER(self);
            ASSERT_VALID_POINTER_PARAMETER(handler);
            CAPI_ENTER
            {
                auto cppServer = reinterpret_cast<ib::sim::rpc::IRpcServer*>(self);
                cppServer->SetCallHandler(MakeRpcCallHandler(context, handler));
                return ib_ReturnCode_SUCCESS;
            }
            CAPI_LEAVE
        }

ib_ReturnCode ib_Rpc_Client_Create(ib_Rpc_Client** out, ib_Participant* participant, const char* controllerName,
                                   const char* functionName, const char* mediaType, const ib_KeyValueList* labels,
                                   void* context, ib_Rpc_CallResultHandler_t resultHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(controllerName);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(mediaType);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpClient = cppParticipant->CreateRpcClient(controllerName, functionName, mediaType, cppLabels,
                                                         MakeRpcCallResultHandler(context, resultHandler));

        *out = reinterpret_cast<ib_Rpc_Client*>(rcpClient);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_Client_Call(ib_Rpc_Client* self, ib_Rpc_CallHandle** outHandle, const ib_ByteVector* argumentData)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_OUT_PARAMETER(outHandle);
    ASSERT_VALID_POINTER_PARAMETER(argumentData);
    CAPI_ENTER
    {
        auto cppClient = reinterpret_cast<ib::sim::rpc::IRpcClient*>(self);
        auto cppCallHandle = cppClient->Call(
            std::vector<uint8_t>(argumentData->data, argumentData->data + argumentData->size));
        *outHandle = reinterpret_cast<ib_Rpc_CallHandle*>(cppCallHandle);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_Client_SetCallResultHandler(ib_Rpc_Client* self, void* context, ib_Rpc_CallResultHandler_t handler)
{
    ASSERT_VALID_POINTER_PARAMETER(self);
    ASSERT_VALID_POINTER_PARAMETER(handler);
    CAPI_ENTER
    {
        auto cppClient = reinterpret_cast<ib::sim::rpc::IRpcClient*>(self);
        cppClient->SetCallResultHandler(MakeRpcCallResultHandler(context, handler));
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_DiscoverServers(ib_Participant* participant, const char* functionName, const char* mediaType,
                                     const ib_KeyValueList* labels, void* context,
                                     ib_Rpc_DiscoveryResultHandler_t resultHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        auto cppMediaType = std::string(mediaType);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppParticipant->DiscoverRpcServers(
            functionName, cppMediaType, cppLabels,
            [resultHandler, context](const std::vector<ib::sim::rpc::RpcDiscoveryResult>& cppDiscoveryResults) {
                ib_Rpc_DiscoveryResultList* results;
                assign(&results, cppDiscoveryResults);
                resultHandler(context, results);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

} // extern "C"