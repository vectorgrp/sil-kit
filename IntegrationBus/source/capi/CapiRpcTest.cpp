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

    MOCK_METHOD1(SetCallResultHandler, void(RpcCallResultHandler handler));
};
class MockRpcServer : public ib::sim::rpc::IRpcServer{
public:
    MOCK_METHOD2(SubmitResult, void(ib::sim::rpc::IRpcCallHandle* callHandle, std::vector<uint8_t> resultData));

    MOCK_METHOD1(SetCallHandler, void(RpcCallHandler handler));
};

class MockParticipant : public ib::mw::test::DummyParticipant
{
public:
    MOCK_METHOD(ib::sim::rpc::IRpcClient*, CreateRpcClient,
                (const std::string& /*controllerName*/, const std::string& /*functionName*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::RpcCallResultHandler /*handler*/),
                (override));

    MOCK_METHOD(ib::sim::rpc::IRpcServer*, CreateRpcServer,
                (const std::string& /*controllerName*/, const std::string& /*functionName*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::RpcCallHandler /*handler*/),
                (override));

    MOCK_METHOD(void, DiscoverRpcServers,
                (const std::string& /*functionName*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/),
                 ib::sim::rpc::RpcDiscoveryResultHandler /*handler*/),
                (override));
};

void Copy_Label(ib_KeyValuePair* dst, const ib_KeyValuePair* src)
{
    auto lenKey = strlen(src->key) + 1;
    auto lenVal = strlen(src->value) + 1;
    dst->key = (const char*)malloc(lenKey);
    dst->value = (const char*)malloc(lenVal);
    if (dst->key == nullptr || dst->value == nullptr)
    {
        throw std::bad_alloc();
    }
    strcpy((char*)dst->key, src->key);
    strcpy((char*)dst->value, src->value);
}

void Create_Labels(ib_KeyValueList** outLabels, const ib_KeyValuePair* labels, uint32_t numLabels)
{
    ib_KeyValueList* newLabels;
    newLabels = (ib_KeyValueList*)malloc(sizeof(ib_KeyValueList));
    if (newLabels == nullptr)
    {
        throw std::bad_alloc();
    }
    newLabels->numLabels = numLabels;
    newLabels->labels = (ib_KeyValuePair*)malloc(numLabels * sizeof(ib_KeyValuePair));
    if (newLabels->labels == nullptr)
    {
        throw std::bad_alloc();
    }
    for (uint32_t i = 0; i < numLabels; i++)
    {
        Copy_Label(&newLabels->labels[i], &labels[i]);
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

void CallHandler(void* /*context*/, ib_Rpc_Server* /*server*/, const ib_Rpc_CallEvent* /*event*/)
{
}

void CallReturnHandler(void* /*context*/, ib_Rpc_Client* /*client*/, const ib_Rpc_CallResultEvent* /*event*/)
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
    EXPECT_CALL(mockParticipant, CreateRpcClient("client", "functionName", testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "functionName", rpcMediaType,
                                      labelList, nullptr, &CallReturnHandler);
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
    EXPECT_CALL(mockParticipant, CreateRpcServer("server", "functionName", testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "functionName", rpcMediaType,
                                      labelList, nullptr, &CallHandler);
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

    returnCode = ib_Rpc_Client_Create(nullptr, (ib_Participant*)&mockParticipant, "client", "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, nullptr, "client", "functionName", mediaType, labelList, dummyContextPtr,
                                      &CallReturnHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, nullptr, "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", nullptr, mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "functionName",
                                      nullptr, labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Client_Create(&client, (ib_Participant*)&mockParticipant, "client", "functionName", mediaType,
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

    returnCode = ib_Rpc_Server_Create(nullptr, (ib_Participant*)&mockParticipant, "server", "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, nullptr, "server", "functionName", mediaType, labelList, dummyContextPtr,
                                      &CallHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", nullptr, mediaType,
                                      labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "functionName",
                                      nullptr, labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    returnCode = ib_Rpc_Server_Create(&server, (ib_Participant*)&mockParticipant, "server", "functionName", mediaType,
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
        ib_Rpc_DiscoverServers(nullptr, "functionName", mediaType, labelList, dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_DiscoverServers((ib_Participant*)&mockParticipant, "functionName", mediaType, labelList,
                                        dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    EXPECT_CALL(mockParticipant, DiscoverRpcServers(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = ib_Rpc_DiscoverServers((ib_Participant*)&mockParticipant, "functionName", mediaType, labelList,
                                        dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

void CallHandlerWithStringContext(void* context, ib_Rpc_Server* /*server*/, const ib_Rpc_CallEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallHandlerWithStringContext";
}

TEST_F(CapiRpcTest, rpc_server_set_call_handler_wraps_c_handler_and_passes_context)
{
    const auto cRpcServer = reinterpret_cast<ib_Rpc_Server*>(&mockRpcServer);

    std::string context;
    EXPECT_CALL(mockRpcServer, SetCallHandler(testing::_))
        .WillOnce(testing::Invoke([this, &context](RpcCallHandler handler) {
            ASSERT_EQ(context, "");
            handler(&mockRpcServer, RpcCallEvent{});
            ASSERT_EQ(context, "CallHandlerWithStringContext");
        }));

    ib_Rpc_Server_SetCallHandler(cRpcServer, &context, CallHandlerWithStringContext);
}

void CallResultHandlerWithStringContext(void* context, ib_Rpc_Client* /*client*/,
                                        const ib_Rpc_CallResultEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallResultHandlerWithStringContext";
}

TEST_F(CapiRpcTest, rpc_client_set_call_result_handler_wraps_c_handler_and_passes_context)
{
    const auto cRpcClient = reinterpret_cast<ib_Rpc_Client*>(&mockRpcClient);

    std::string context;
    EXPECT_CALL(mockRpcClient, SetCallResultHandler(testing::_))
        .WillOnce(testing::Invoke([this, &context](RpcCallResultHandler handler) {
            ASSERT_EQ(context, "");
            handler(&mockRpcClient, RpcCallResultEvent{});
            ASSERT_EQ(context, "CallResultHandlerWithStringContext");
        }));

    ib_Rpc_Client_SetCallResultHandler(cRpcClient, &context, CallResultHandlerWithStringContext);
}

}
