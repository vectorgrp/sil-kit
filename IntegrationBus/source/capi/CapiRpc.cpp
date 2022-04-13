#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/rpc/all.hpp"

#include "CapiImpl.h"
#include "TypeConversion.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>

static void assign(ib_Rpc_DiscoveryResultList** cResultList, const std::vector<ib::sim::rpc::RpcDiscoveryResult>& cppDiscoveryResults)
{
    size_t numResults = cppDiscoveryResults.size();
    size_t resultsMemSize = sizeof(ib_Rpc_DiscoveryResultList) + (numResults * sizeof(ib_Rpc_DiscoveryResult));
    *cResultList = (ib_Rpc_DiscoveryResultList*)malloc(resultsMemSize);
    (*cResultList)->numResults = numResults;

    uint32_t i = 0;
    for (auto&& r : cppDiscoveryResults)
    {
        (*cResultList)->results[i].interfaceId = ib_InterfaceIdentifier_RpcDiscoveryResult;
        (*cResultList)->results[i].functionName = r.functionName.c_str();
        (*cResultList)->results[i].exchangeFormat =
            new ib_Rpc_ExchangeFormat{ib_InterfaceIdentifier_RpcExchangeFormat, r.exchangeFormat.mediaType.c_str()};
        assign(&(*cResultList)->results[i].labelList, r.labels);
        i++;
    };
}

extern "C" {

ib_ReturnCode ib_Rpc_Client_Create(ib_Rpc_Client** out, ib_Participant* participant,
                                       const char* functionName, ib_Rpc_ExchangeFormat* exchangeFormat,
                                       const ib_KeyValueList* labels, void* context,
                                       ib_Rpc_ResultHandler_t resultHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(exchangeFormat);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        std::string strFunctionName(functionName);
        ib::sim::rpc::RpcExchangeFormat cppExchangeFormat{std::string(exchangeFormat->mediaType)};
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpClient = cppParticipant->CreateRpcClient(
            functionName, cppExchangeFormat, cppLabels,
            [resultHandler, context] (ib::sim::rpc::IRpcClient* cppClient, ib::sim::rpc::IRpcCallHandle* callHandle,
                  const ib::sim::rpc::CallStatus callStatus, const std::vector<uint8_t>& returnData) 
            {
                auto* cClient = reinterpret_cast<ib_Rpc_Client*>(cppClient);
                uint8_t* payloadPointer = NULL;
                if (returnData.size() > 0)
                {
                    payloadPointer = (uint8_t*)&(returnData[0]);
                }
                const ib_ByteVector cReturnData{payloadPointer, returnData.size()};
                ib_Rpc_CallHandle* cCallHandle = reinterpret_cast<ib_Rpc_CallHandle*>(callHandle);
                ib_Rpc_CallStatus cCallStatus = (ib_Rpc_CallStatus)callStatus;
                resultHandler(context, cClient, cCallHandle, cCallStatus, &cReturnData);
            });

        *out = reinterpret_cast<ib_Rpc_Client*>(rcpClient);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_Server_Create(ib_Rpc_Server** out, ib_Participant* participant, const char* functionName,
                                   ib_Rpc_ExchangeFormat* exchangeFormat, const ib_KeyValueList* labels, void* context,
                                   ib_Rpc_CallHandler_t callHandler)
{
    ASSERT_VALID_OUT_PARAMETER(out);
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_POINTER_PARAMETER(functionName);
    ASSERT_VALID_POINTER_PARAMETER(exchangeFormat);
    ASSERT_VALID_HANDLER_PARAMETER(callHandler);
    CAPI_ENTER
    {
        std::string strFunctionName(functionName);
        ib::sim::rpc::RpcExchangeFormat cppExchangeFormat{std::string(exchangeFormat->mediaType)};
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        auto rcpServer = cppParticipant->CreateRpcServer(
            functionName, cppExchangeFormat, cppLabels,
            [callHandler, context](ib::sim::rpc::IRpcServer* cppServer, ib::sim::rpc::IRpcCallHandle* callHandle,
                  const std::vector<uint8_t>& argumentData)
            {
                auto* cServer = reinterpret_cast<ib_Rpc_Server*>(cppServer);
                uint8_t* payloadPointer = NULL;
                if (argumentData.size() > 0)
                {
                    payloadPointer = (uint8_t*)&(argumentData[0]);
                }
                const ib_ByteVector cArgumentData{payloadPointer, argumentData.size()};
                ib_Rpc_CallHandle* cCallHandle = reinterpret_cast<ib_Rpc_CallHandle*>(callHandle);
                callHandler(context, cServer, cCallHandle, &cArgumentData);
            });

        *out = reinterpret_cast<ib_Rpc_Server*>(rcpServer);
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
            std::vector<uint8_t>(&(argumentData->data[0]), &(argumentData->data[0]) + argumentData->size));
        *outHandle = reinterpret_cast<ib_Rpc_CallHandle*>(cppCallHandle);
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
            std::vector<uint8_t>(&(returnData->data[0]), &(returnData->data[0]) + returnData->size);
        cppServer->SubmitResult(cppCallHandle, cppReturnData);
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}

ib_ReturnCode ib_Rpc_DiscoverServers(ib_Participant* participant, const char* functionName,
                                     ib_Rpc_ExchangeFormat* exchangeFormat, const ib_KeyValueList* labels,
                                     void* context, ib_Rpc_DiscoveryResultHandler_t resultHandler)
{
    ASSERT_VALID_POINTER_PARAMETER(participant);
    ASSERT_VALID_HANDLER_PARAMETER(resultHandler);
    CAPI_ENTER
    {
        auto cppParticipant = reinterpret_cast<ib::mw::IParticipant*>(participant);
        std::string cppFunctionName(functionName);
        ib::sim::rpc::RpcExchangeFormat cppExchangeFormat{std::string(exchangeFormat->mediaType)};
        std::map<std::string, std::string> cppLabels;
        assign(cppLabels, labels);
        cppParticipant->DiscoverRpcServers(
            cppFunctionName, cppExchangeFormat, cppLabels,
            [resultHandler, context](const std::vector<ib::sim::rpc::RpcDiscoveryResult>& cppDiscoveryResults) {
                ib_Rpc_DiscoveryResultList* results;
                assign(&results, cppDiscoveryResults);
                resultHandler(context, results);
            });
        return ib_ReturnCode_SUCCESS;
    }
    CAPI_LEAVE
}


}