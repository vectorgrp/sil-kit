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
    MOCK_METHOD(void, Call, (SilKit::Util::Span<const uint8_t> data, void* userContext), (override));
    MOCK_METHOD(void, CallWithTimeout, (SilKit::Util::Span<const uint8_t> data, std::chrono::nanoseconds timeout, void* userContext), (override));

    MOCK_METHOD1(SetCallResultHandler, void(RpcCallResultHandler handler));
};
class MockRpcServer : public SilKit::Services::Rpc::IRpcServer{
public:
    MOCK_METHOD(void, SubmitResult, (SilKit::Services::Rpc::IRpcCallHandle* callHandle, SilKit::Util::Span<const uint8_t> resultData), (override));

    MOCK_METHOD1(SetCallHandler, void(RpcCallHandler handler));
};

class MockParticipant : public SilKit::Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(SilKit::Services::Rpc::IRpcClient*, CreateRpcClient,
                (const std::string& /*controllerName*/, const SilKit::Services::Rpc::RpcSpec& /*rpcSpec*/,
                 SilKit::Services::Rpc::RpcCallResultHandler /*handler*/),
                (override));

    MOCK_METHOD(SilKit::Services::Rpc::IRpcServer*, CreateRpcServer,
                (const std::string& /*controllerName*/, const SilKit::Services::Rpc::RpcSpec& /*rpcSpec*/,
                 SilKit::Services::Rpc::RpcCallHandler /*handler*/),
                (override));
};

class Test_CapiRpc : public testing::Test
{
public:
    MockRpcClient mockRpcClient;
    MockRpcServer mockRpcServer;
    MockParticipant mockParticipant;
    Test_CapiRpc()
    {
        dummyCallHandle = std::make_unique<RpcCallHandle>(SilKit::Util::Uuid{ 1, 1 });
        callHandlePtr = dummyCallHandle.get();
        callHandle = reinterpret_cast<SilKit_RpcCallHandle*>(callHandlePtr);

        mediaType = "A";

        dummyContext.someInt = 1234;
        dummyContextPtr = (void*)&dummyContext;
    }

    std::unique_ptr<RpcCallHandle> dummyCallHandle;
    IRpcCallHandle* callHandlePtr;
    SilKit_RpcCallHandle* callHandle;

    typedef struct
    {
        uint32_t someInt;
    } TransmitContext;

    TransmitContext dummyContext;
    void* dummyContextPtr;

    const char* mediaType;

};

void SilKitCALL CallHandler(void* /*context*/, SilKit_RpcServer* /*server*/, const SilKit_RpcCallEvent* /*event*/)
{
}

void SilKitCALL CallReturnHandler(void* /*context*/, SilKit_RpcClient* /*client*/, const SilKit_RpcCallResultEvent* /*event*/)
{
}

TEST_F(Test_CapiRpc, rpc_client_function_mapping)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcClient* client;
    SilKit_RpcSpec rpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, rpcSpec);
    rpcSpec.functionName = "TopicA";
    rpcSpec.mediaType = mediaType;
    rpcSpec.labelList.numLabels = 0;
    rpcSpec.labelList.labels = 0;

    EXPECT_CALL(mockParticipant, CreateRpcClient("client", testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", &rpcSpec, nullptr,
                                         &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = { 0, 0 };

    EXPECT_CALL(mockRpcClient, Call(testing::_, nullptr)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &data, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    const auto userContext = reinterpret_cast<void*>(uintptr_t{12345});

    EXPECT_CALL(mockRpcClient, Call(testing::_, userContext)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &data, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockRpcClient, CallWithTimeout(testing::_, std::chrono::nanoseconds{123456}, userContext))
        .Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_CallWithTimeout((SilKit_RpcClient*)&mockRpcClient, &data, 123456, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiRpc, rpc_server_function_mapping)
{
    SilKit_ReturnCode returnCode;

    
    SilKit_RpcSpec rpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, rpcSpec);
    rpcSpec.functionName = "TopicA";
    rpcSpec.mediaType = mediaType;
    rpcSpec.labelList.numLabels = 0;
    rpcSpec.labelList.labels = 0;

    SilKit_RpcServer* server;
    EXPECT_CALL(mockParticipant, CreateRpcServer("server", testing::_, testing::_))
        .Times(testing::Exactly(1));
    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", &rpcSpec, nullptr,
                                         &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    SilKit_ByteVector data = {0, 0};
    EXPECT_CALL(mockRpcServer, SubmitResult(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiRpc, rpc_client_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcClient* client;

    SilKit_RpcSpec rpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, rpcSpec);
    rpcSpec.functionName = "TopicA";
    rpcSpec.mediaType = mediaType;
    rpcSpec.labelList.numLabels = 0;
    rpcSpec.labelList.labels = 0;

    returnCode = SilKit_RpcClient_Create(nullptr, (SilKit_Participant*)&mockParticipant, "client", &rpcSpec,
                                         dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, nullptr, "client", &rpcSpec, dummyContextPtr,
                                      &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, nullptr, &rpcSpec,
                                         dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", nullptr, dummyContextPtr, &CallReturnHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Create(&client, (SilKit_Participant*)&mockParticipant, "client", &rpcSpec,
                                         dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ByteVector data = {0, 0};
    const auto userContext = reinterpret_cast<void*>(uintptr_t(12345));

    returnCode = SilKit_RpcClient_Call(nullptr, &data, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Call(nullptr, &data, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, nullptr, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_CallWithTimeout(nullptr, &data, 987654321, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcClient_CallWithTimeout((SilKit_RpcClient*)&mockRpcClient, nullptr, 987654321, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiRpc, rpc_server_bad_parameters)
{
    SilKit_ReturnCode returnCode;
    SilKit_RpcServer* server;

    SilKit_RpcSpec rpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, rpcSpec);
    rpcSpec.functionName = "TopicA";
    rpcSpec.mediaType = mediaType;
    rpcSpec.labelList.numLabels = 0;
    rpcSpec.labelList.labels = 0;

    returnCode = SilKit_RpcServer_Create(nullptr, (SilKit_Participant*)&mockParticipant, "server", &rpcSpec,
                                         dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, nullptr, "server", &rpcSpec, dummyContextPtr,
                                      &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", nullptr, dummyContextPtr, &CallHandler);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_RpcServer_Create(&server, (SilKit_Participant*)&mockParticipant, "server", &rpcSpec,
                                         dummyContextPtr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    SilKit_ByteVector     data = {0, 0};
    returnCode = SilKit_RpcServer_SubmitResult(nullptr, callHandle, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, nullptr, &data);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_RpcServer_SubmitResult((SilKit_RpcServer*)&mockRpcServer, callHandle, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}


TEST_F(Test_CapiRpc, rpc_client_call)
{
    SilKit_ReturnCode returnCode = 0;
    // create payload
    uint8_t buffer[64];
    int messageCounter = 1;
    size_t payloadSize = snprintf((char*)buffer, sizeof(buffer), "RPC %i", messageCounter);
    SilKit_ByteVector data = { &buffer[0], payloadSize };

    std::vector<uint8_t> refData(&(data.data[0]), &(data.data[0]) + data.size);

    const auto userContext = reinterpret_cast<void*>(uintptr_t(12345));

    EXPECT_CALL(mockRpcClient, Call(PayloadMatcher(refData), userContext)).Times(testing::Exactly(1));
    returnCode = SilKit_RpcClient_Call((SilKit_RpcClient*)&mockRpcClient, &data, userContext);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiRpc, rpc_server_submit)
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

void SilKitCALL CallHandlerWithStringContext(void* context, SilKit_RpcServer* /*server*/, const SilKit_RpcCallEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallHandlerWithStringContext";
}

TEST_F(Test_CapiRpc, rpc_server_set_call_handler_wraps_c_handler_and_passes_context)
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

void SilKitCALL CallResultHandlerWithStringContext(void* context, SilKit_RpcClient* /*client*/,
                                        const SilKit_RpcCallResultEvent* /*event*/)
{
    if (context == nullptr)
    {
        return;
    }

    *static_cast<std::string*>(context) = "CallResultHandlerWithStringContext";
}

TEST_F(Test_CapiRpc, rpc_client_set_call_result_handler_wraps_c_handler_and_passes_context)
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
