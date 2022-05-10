/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/rpc/all.hpp"
#include "MockParticipant.hpp"
#include "RpcCallHandle.hpp"

namespace {
using namespace ib::sim::rpc;
using ib::mw::test::DummyParticipant;

MATCHER_P(PayloadMatcher, controlPayload, "") {
    *result_listener << "matches data payloads by their content and length";
    if (arg.size() != controlPayload.size()) {
        return false;
    }
    for (size_t i = 0; i < arg.size(); i++) {
        if (arg[i] != controlPayload[i]) {
            return false;
        }
    }
    return true;
}

class MockRpcClient : public ib::sim::rpc::IRpcClient {
public:
    MOCK_METHOD1(Call, ib::sim::rpc::IRpcCallHandle*(std::vector<uint8_t> data));
    virtual ib::sim::rpc::IRpcCallHandle* Call(const uint8_t* /*data*/, std::size_t /*size*/) { return nullptr; };

    MOCK_METHOD1(SetCallReturnHandler,
                 void(std::function<void(ib::sim::rpc::IRpcClient* client, IRpcCallHandle* callHandle, const ib::sim::rpc::CallStatus callStatus,
                     const std::vector<uint8_t>& returnData)> handler));
};
class MockRpcServer : public ib::sim::rpc::IRpcServer{
public:
    MOCK_METHOD2(SubmitResult, void(ib::sim::rpc::IRpcCallHandle* callHandle, std::vector<uint8_t> resultData));

    MOCK_METHOD1(SetRpcHandler,
                 void(std::function<void(ib::sim::rpc::IRpcServer* server, ib::sim::rpc::IRpcCallHandle* callHandle,
                                         const std::vector<uint8_t>& argumentData)>
                     handler));
};

class MockParticipant : public ib::mw::test::DummyParticipant
{
public:
    MOCK_METHOD(ib::sim::rpc::IRpcClient*, CreateRpcClient,
                (const std::string& /*controllerName*/, const std::string& /*rpcChannel*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::CallReturnHandler /*handler*/),
                (override));

    MOCK_METHOD(ib::sim::rpc::IRpcServer*, CreateRpcServer,
                (const std::string& /*controllerName*/, const std::string& /*rpcChannel*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::CallProcessor /*handler*/),
                (override));

    MOCK_METHOD(void, DiscoverRpcServers,
                (const std::string& /*rpcChannel*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::DiscoveryResultHandler /*handler*/),
                (override));
};

void Copy_Label(ib_KeyValuePair* dst, const ib_KeyValuePair* src)
{
    auto lenKey = strlen(src->key) + 1;
    auto lenVal = strlen(src->value) + 1;
    dst->key = (const char*)malloc(lenKey);
    dst->value = (const char*)malloc(lenVal);
    if (dst->key != nullptr && dst->value != nullptr)
    {
        strcpy((char*)dst->key, src->key);
        strcpy((char*)dst->value, src->value);
    }
}

void Create_Labels(ib_KeyValueList** outLabels, const ib_KeyValuePair* labels, uint32_t numLabels)
{
    ib_KeyValueList* newLabels;
    size_t labelsSize = numLabels * sizeof(ib_KeyValuePair);
    size_t labelListSize = sizeof(ib_KeyValueList) + labelsSize;
    newLabels = (ib_KeyValueList*)malloc(labelListSize);
    if (newLabels != nullptr)
    {
        newLabels->numLabels = numLabels;
        for (uint32_t i = 0; i < numLabels; i++)
        {
            Copy_Label(&newLabels->labels[i], &labels[i]);
        }
    }
    *outLabels = newLabels;
}

class CapiRpcTest : public testing::Test
{
public: 
    MockRpcClient mockRpcClient;
    MockRpcServer mockRpcServer;
    MockParticipant mockParticipant;
    CapiRpcTest()
    { 
        dummyCallHandle = std::make_unique<CallHandleImpl>(ib::sim::rpc::CallUUID{ 1, 1 });
        callHandlePtr = dummyCallHandle.get();
        callHandle = reinterpret_cast<ib_Rpc_CallHandle*>(callHandlePtr);

        uint32_t numLabels = 1;
        ib_KeyValuePair labels[1] = {{"KeyA", "ValA"}};
        Create_Labels(&labelList, labels, numLabels);

        mediaType = "A";

        dummyContext.someInt = 1234;
        dummyContextPtr = (void*)&dummyContext;
    }

    std::unique_ptr<CallHandleImpl> dummyCallHandle;
    IRpcCallHandle* callHandlePtr;
    ib_Rpc_CallHandle* callHandle;

    typedef struct
    {
        uint32_t someInt;
    } TransmitContext;

    TransmitContext dummyContext;
    void* dummyContextPtr;

    const char* mediaType;
    ib_KeyValueList* labelList;

};

void RpcHandler(void* /*context*/, ib_Rpc_Server* /*server*/, ib_Rpc_CallHandle* /*callHandle*/, const ib_ByteVector* /*argumentData*/)
{
}

void CallResultHandler(void* /*context*/, ib_Rpc_Client* /*client*/, ib_Rpc_CallHandle* /*callHandle*/,
                       ib_Rpc_CallStatus /*callStatus*/, const ib_ByteVector* /*returnData*/)
{
}

void DiscoveryResultHandler(void* /*context*/, const ib_Rpc_DiscoveryResultList* /*discoveryResults*/)
{
}

TEST_F(CapiRpcTest, rpc_client_function_mapping)
{
    ib_ReturnCode returnCode;
    ib_Rpc_Client* client;
    const char* rpcMediaType = "A";
    EXPECT_CALL(mockParticipant, CreateRpcClient("client", "rpcChannel", testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "rpcChannel", rpcMediaType,
                                      labelList, nullptr, &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    ib_ByteVector data = { 0, 0 };
    EXPECT_CALL(mockRpcClient, Call(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Call((ib_Rpc_Client*)&mockRpcClient, &callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_server_function_mapping)
{
    ib_ReturnCode returnCode;

    ib_Rpc_Server* server;
    const char* rpcMediaType = "A";
    EXPECT_CALL(mockParticipant, CreateRpcServer("server", "rpcChannel", testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "rpcChannel", rpcMediaType,
                                      labelList, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    
    ib_ByteVector data = {0, 0};
    EXPECT_CALL(mockRpcServer, SubmitResult(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_SubmitResult((ib_Rpc_Server*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_client_bad_parameters)
{
    ib_ReturnCode returnCode;
    ib_Rpc_Client* client;

    returnCode = ib_Rpc_Client_Create(nullptr, (ib_Participant*)&mockParticipant, "client", "rpcChannel", mediaType,
                                      labelList, dummyContextPtr, &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, nullptr, "client", "rpcChannel", mediaType, labelList, dummyContextPtr,
                                      &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, nullptr, "rpcChannel", mediaType,
                                      labelList, dummyContextPtr, &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", nullptr, mediaType,
                                      labelList, dummyContextPtr, &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "rpcChannel",
                                      nullptr, labelList, dummyContextPtr, &CallResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "rpcChannel", mediaType,
                                      labelList, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_ByteVector data = {0, 0};
    returnCode = ib_Rpc_Client_Call(nullptr, &callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Call((ib_Rpc_Client*)&mockRpcClient, nullptr, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Call((ib_Rpc_Client*)&mockRpcClient, &callHandle, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}

TEST_F(CapiRpcTest, rpc_server_bad_parameters)
{
    ib_ReturnCode returnCode;
    ib_Rpc_Server* server;

    returnCode = ib_Rpc_Server_Create(nullptr, (ib_Participant*)&mockParticipant, "server", "rpcChannel", mediaType,
                                      labelList, dummyContextPtr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, nullptr, "server", "rpcChannel", mediaType, labelList, dummyContextPtr,
                                      &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", nullptr, mediaType,
                                      labelList, dummyContextPtr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "rpcChannel",
                                      nullptr, labelList, dummyContextPtr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "rpcChannel", mediaType,
                                      labelList, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_ByteVector     data = {0, 0};
    returnCode = ib_Rpc_Server_SubmitResult(nullptr, callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_SubmitResult((ib_Rpc_Server*)&mockRpcServer, nullptr, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_SubmitResult((ib_Rpc_Server*)&mockRpcServer, callHandle, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}


TEST_F(CapiRpcTest, rpc_client_call)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "RPC %i", messageCounter);
    ib_ByteVector data = { &buffer[0], payloadSize };

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockRpcClient, Call(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Call((ib_Rpc_Client*)&mockRpcClient, &callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_server_submit)
{
    ib_ReturnCode returnCode = 0;
    // create payload
    uint8_t       buffer[64];
    int           messageCounter = 1;
    size_t        payloadSize = snprintf((char*)buffer, sizeof(buffer), "RPC %i", messageCounter);
    ib_ByteVector data = {&buffer[0], payloadSize};

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockRpcServer, SubmitResult(callHandlePtr, PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_SubmitResult((ib_Rpc_Server*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_discovery_query)
{
    ib_ReturnCode returnCode = 0;
    returnCode =
        ib_Rpc_DiscoverServers(nullptr, "rpcChannel", mediaType, labelList, dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_DiscoverServers((ib_Participant*)&mockParticipant, "rpcChannel", mediaType, labelList,
                                        dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    EXPECT_CALL(mockParticipant, DiscoverRpcServers(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = ib_Rpc_DiscoverServers((ib_Participant*)&mockParticipant, "rpcChannel", mediaType, labelList,
                                        dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

}
