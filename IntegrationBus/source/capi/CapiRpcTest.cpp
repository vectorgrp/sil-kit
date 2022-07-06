// Copyright (c) Vector Informatik GmbH. All rights reserved.

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/rpc/all.hpp"
#include "MockParticipant.hpp"
#include "RpcCallHandle.hpp"

namespace {
using namespace SilKit::Services::Rpc;
using SilKit::Core::Tests::DummyParticipant;

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

class MockRpcClient : public SilKit::Services::Rpc::IRpcClient {
public:
    MOCK_METHOD1(Call, SilKit::Services::Rpc::IRpcCallHandle*(std::vector<uint8_t> data));
    virtual SilKit::Services::Rpc::IRpcCallHandle* Call(const uint8_t* /*data*/, std::size_t /*size*/) { return nullptr; };

    MOCK_METHOD1(SetCallResultHandler, void(RpcCallResultHandler handler));
};
class MockRpcServer : public SilKit::Services::Rpc::IRpcServer{
public:
    MOCK_METHOD2(SubmitResult, void(SilKit::Services::Rpc::IRpcCallHandle* callHandle, std::vector<uint8_t> resultData));

    MOCK_METHOD1(SetCallHandler, void(RpcCallHandler handler));
};

class MockParticipant : public SilKit::Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(SilKit::Services::Rpc::IRpcClient*, CreateRpcClient,
                (const std::string& /*controllerName*/, const std::string& /*functionName*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 SilKit::Services::Rpc::RpcCallResultHandler /*handler*/),
                (override));

    MOCK_METHOD(SilKit::Services::Rpc::IRpcServer*, CreateRpcServer,
                (const std::string& /*controllerName*/, const std::string& /*functionName*/,
                 const std::string& /*mediaType*/, (const std::map<std::string, std::string>& /*labels*/),
                 SilKit::Services::Rpc::RpcCallHandler /*handler*/),
                (override));

    MOCK_METHOD(void, DiscoverRpcServers,
                (const std::string& /*functionName*/, const std::string& /*mediaType*/,
                 (const std::map<std::string, std::string>& /*labels*/),
                 SilKit::Services::Rpc::RpcDiscoveryResultHandler /*handler*/),
                (override));
};

void Copy_Label(SilKit_KeyValuePair* dst, const SilKit_KeyValuePair* src)
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

void Create_Labels(SilKit_KeyValueList** outLabels, const SilKit_KeyValuePair* labels, uint32_t numLabels)
{
    SilKit_KeyValueList* newLabels;
    newLabels = (SilKit_KeyValueList*)malloc(sizeof(SilKit_KeyValueList));
    if (newLabels == nullptr)
    {
        throw std::bad_alloc();
    }
    newLabels->numLabels = numLabels;
    newLabels->labels = (SilKit_KeyValuePair*)malloc(numLabels * sizeof(SilKit_KeyValuePair));
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
        dummyCallHandle = std::make_unique<CallHandleImpl>(SilKit::Services::Rpc::CallUUID{ 1, 1 });
        callHandlePtr = dummyCallHandle.get();
        callHandle = reinterpret_cast<SilKit_CallHandle*>(callHandlePtr);

        uint32_t numLabels = 1;
        SilKit_KeyValuePair labels[1] = {{"KeyA", "ValA"}};
        Create_Labels(&labelList, labels, numLabels);

        mediaType = "A";

        dummyContext.someInt = 1234;
        dummyContextPtr = (void*)&dummyContext;
    }

    std::unique_ptr<CallHandleImpl> dummyCallHandle;
    IRpcCallHandle* callHandlePtr;
    SilKit_CallHandle* callHandle;

    typedef struct
    {
        uint32_t someInt;
    } TransmitContext;

    TransmitContext dummyContext;
    void* dummyContextPtr;

    const char* mediaType;
    SilKit_KeyValueList* labelList;

};

void CallHandler(void* /*context*/, SilKit_RpcServer* /*server*/, const SilKit_CallEvent* /*event*/)
{
}

void CallReturnHandler(void* /*context*/, SilKit_RpcClient* /*client*/, const SilKit_CallResultEvent* /*event*/)
{
}

void DiscoveryResultHandler(void* /*context*/, const SilKit_DiscoveryResultList* /*discoveryResults*/)
{
}

TEST_F(CapiRpcTest, rpc_client_function_mapping)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcClient* client;
    const char* rpcMediaType = "A";
    EXPECT_CALL(mockParticipant, CreateRpcClient("client", "functionName", testing::_, testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", "functionName", rpcMediaType,
                                      labelList, nullptr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = { 0, 0 };
    EXPECT_CALL(mockRpcClient, Call(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_server_function_mapping)
{
    SilKit_ReturnCode returnCode;

    SilKit_RpcServer* server;
    const char* rpcMediaType = "A";
    EXPECT_CALL(mockParticipant, CreateRpcServer("server", "functionName", testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", "functionName", rpcMediaType,
                                      labelList, nullptr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = {0, 0};
    EXPECT_CALL(mockRpcServer, SubmitResult(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_client_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcClient* client;

    returnCode = SilKit_RpcClient_Create(nullptr, (SilKit_Participant*)&mockParticipant, "client", "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, nullptr, "client", "functionName", mediaType, labelList, dummyContextPtr,
                                      &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, nullptr, "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", nullptr, mediaType,
                                      labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", "functionName",
                                      nullptr, labelList, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", "functionName", mediaType,
                                      labelList, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ByteVector data = {0, 0};
    returnCode = SilKit_RpcClient_Call(nullptr, &callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, nullptr, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &callHandle, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(CapiRpcTest, rpc_server_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcServer* server;

    returnCode = SilKit_RpcServer_Create(nullptr, (SilKit_Participant*)&mockParticipant, "server", "functionName", mediaType,
                                      labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, nullptr, "server", "functionName", mediaType, labelList, dummyContextPtr,
                                      &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", nullptr, mediaType,
                                      labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", "functionName",
                                      nullptr, labelList, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", "functionName", mediaType,
                                      labelList, dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ByteVector     data = {0, 0};
    returnCode = SilKit_RpcServer_SubmitResult(nullptr, callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, nullptr, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, callHandle, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}


TEST_F(CapiRpcTest, rpc_client_call)
{
    SilKit_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "RPC %i", messageCounter);
    SilKit_ByteVector data = { &buffer[0], payloadSize };

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockRpcClient, Call(PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_server_submit)
{
    SilKit_ReturnCode returnCode = 0;
    // create payload
    uint8_t       buffer[64];
    int           messageCounter = 1;
    size_t        payloadSize = snprintf((char*)buffer, sizeof(buffer), "RPC %i", messageCounter);
    SilKit_ByteVector data = {&buffer[0], payloadSize};

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);
    EXPECT_CALL(mockRpcServer, SubmitResult(callHandlePtr, PayloadMatcher(refData))).Times(testing::Exactly(1));
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_discovery_query)
{
    SilKit_ReturnCode returnCode = 0;
    returnCode =
        SilKit_DiscoverServers(nullptr, "functionName", mediaType, labelList, dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_DiscoverServers((SilKit_Participant*)&mockParticipant, "functionName", mediaType, labelList,
                                        dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    EXPECT_CALL(mockParticipant, DiscoverRpcServers(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = SilKit_DiscoverServers((SilKit_Participant*)&mockParticipant, "functionName", mediaType, labelList,
                                        dummyContextPtr, &DiscoveryResultHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

void CallHandlerWithStringContext(void* context, SilKit_RpcServer* /*server*/, const SilKit_CallEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallHandlerWithStringContext";
}

TEST_F(CapiRpcTest, rpc_server_set_call_handler_wraps_c_handler_and_passes_context)
{
    const auto cRpcServer = reinterpret_cast<SilKit_RpcServer*>(&mockRpcServer);

    std::string context;
    EXPECT_CALL(mockRpcServer, SetCallHandler(testing::_))
        .WillOnce(testing::Invoke([this, &context](RpcCallHandler handler) {
            ASSERT_EQ(context, "");
            handler(&mockRpcServer, RpcCallEvent{});
            ASSERT_EQ(context, "CallHandlerWithStringContext");
        }));

    SilKit_RpcServer_SetCallHandler(cRpcServer, &context, CallHandlerWithStringContext);
}

void CallResultHandlerWithStringContext(void* context, SilKit_RpcClient* /*client*/,
                                        const SilKit_CallResultEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallResultHandlerWithStringContext";
}

TEST_F(CapiRpcTest, rpc_client_set_call_result_handler_wraps_c_handler_and_passes_context)
{
    const auto cRpcClient = reinterpret_cast<SilKit_RpcClient*>(&mockRpcClient);

    std::string context;
    EXPECT_CALL(mockRpcClient, SetCallResultHandler(testing::_))
        .WillOnce(testing::Invoke([this, &context](RpcCallResultHandler handler) {
            ASSERT_EQ(context, "");
            handler(&mockRpcClient, RpcCallResultEvent{});
            ASSERT_EQ(context, "CallResultHandlerWithStringContext");
        }));

    SilKit_RpcClient_SetCallResultHandler(cRpcClient, &context, CallResultHandlerWithStringContext);
}

} //namespace
