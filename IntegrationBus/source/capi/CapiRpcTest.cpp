/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/rpc/all.hpp"
#include "ib/cfg/Config.hpp"
#include "MockComAdapter.hpp"
#include "RpcCallHandle.hpp"

namespace {
using namespace ib::sim::rpc;
using ib::mw::test::DummyComAdapter;

MATCHER_P(PayloadMatcher, controlPayload, "matches data payloads by their content and length") {
    if (arg.size() != controlPayload.size()) {
        return false;
    }
    for (int i = 0; i < arg.size(); i++) {
        if (arg[i] != controlPayload[i]) {
            return false;
        }
    }
    return true;
}

class MockRpcClient : public ib::sim::rpc::IRpcClient {
public:
    auto Config() const -> const ib::cfg::RpcPort&
    {
        static const ib::cfg::RpcPort portCfg{};
        return portCfg;
    };
    MOCK_METHOD1(Call, ib::sim::rpc::IRpcCallHandle*(std::vector<uint8_t> data));
    virtual ib::sim::rpc::IRpcCallHandle* Call(const uint8_t* data, std::size_t size) { return nullptr; };

    MOCK_METHOD1(SetCallReturnHandler,
                 void(std::function<void(ib::sim::rpc::IRpcClient* client, IRpcCallHandle* callHandle, const ib::sim::rpc::CallStatus callStatus,
                     const std::vector<uint8_t>& returnData)> handler));
};
class MockRpcServer : public ib::sim::rpc::IRpcServer{
public:
    auto Config() const -> const ib::cfg::RpcPort&
    {
        static const ib::cfg::RpcPort portCfg{};
        return portCfg;
    };
    MOCK_METHOD2(SubmitResult, void(ib::sim::rpc::IRpcCallHandle* callHandle, std::vector<uint8_t> resultData));

    MOCK_METHOD1(SetRpcHandler,
                 void(std::function<void(ib::sim::rpc::IRpcServer* server, ib::sim::rpc::IRpcCallHandle* callHandle,
                                         const std::vector<uint8_t>& argumentData)>
                     handler));
};

class MockComAdapter : public ib::mw::test::DummyComAdapter
{
  public:
    MOCK_METHOD(ib::sim::rpc::IRpcClient*, CreateRpcClient,
                (const std::string& /*functionName*/, const ib::sim::rpc::RpcExchangeFormat /*exchangeFormat*/,
                 ib::sim::rpc::CallReturnHandler /*handler*/), (override));

    MOCK_METHOD(ib::sim::rpc::IRpcServer*, CreateRpcServer,
                (const std::string& /*canonicalName*/, 
                 const ib::sim::rpc::RpcExchangeFormat /*rpcExchangeFormat*/,
                 ib::sim::rpc::CallProcessor /*handler*/), (override));

};

class CapiRpcTest : public testing::Test
{
public: 
    MockRpcClient mockRpcClient;
    MockRpcServer mockRpcServer;
    MockComAdapter mockSimulationParticipant;
    CapiRpcTest()
    { 
        dummyCallHandle = std::make_unique<CallHandleImpl>(ib::sim::rpc::CallUUID{ 1, 1 });
        callHandlePtr = dummyCallHandle.get();
        callHandle = reinterpret_cast<ib_Rpc_CallHandle*>(callHandlePtr);
    }

    std::unique_ptr<CallHandleImpl> dummyCallHandle;
    IRpcCallHandle* callHandlePtr;
    ib_Rpc_CallHandle* callHandle;

};

void RpcHandler(void* context, ib_Rpc_Server* server, ib_Rpc_CallHandle* callHandle,  const ib_ByteVector* argumentData) {

}

void ResultHandler(void* context, ib_Rpc_Client* client, ib_Rpc_CallHandle* callHandle, ib_Rpc_CallStatus callStatus,
                   const ib_ByteVector* returnData)
{
}

TEST_F(CapiRpcTest, rpc_client_function_mapping)
{
    ib_ReturnCode returnCode;
    ib_Rpc_Client* client;
    ib_Rpc_ExchangeFormat rpcExchangeFormat;
    rpcExchangeFormat.mediaType = "A";
    EXPECT_CALL(mockSimulationParticipant, CreateRpcClient).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Create(&client, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, &ResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

    ib_ByteVector data = { 0, 0 };
    ib_Rpc_CallHandle* callHandle;
    EXPECT_CALL(mockRpcClient, Call(testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Client_Call((ib_Rpc_Client*)&mockRpcClient, &callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_server_function_mapping)
{
    ib_ReturnCode returnCode;

    ib_Rpc_Server* server;
    ib_Rpc_ExchangeFormat rpcExchangeFormat;
    rpcExchangeFormat.mediaType = "A";
    EXPECT_CALL(mockSimulationParticipant, CreateRpcServer).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_Create(&server, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    
    ib_ByteVector data = {0, 0};
    EXPECT_CALL(mockRpcServer, SubmitResult(testing::_, testing::_)).Times(testing::Exactly(1));
    returnCode = ib_Rpc_Server_SubmitResult((ib_Rpc_Server*)&mockRpcServer, callHandle, &data);
    EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiRpcTest, rpc_client_bad_parameters)
{
    ib_ReturnCode returnCode;
    ib_Rpc_ExchangeFormat rpcExchangeFormat;
    rpcExchangeFormat.mediaType = "A";
    ib_Rpc_Client* client;

    returnCode = ib_Rpc_Client_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, &ResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Create(&client, nullptr, "functionName", &rpcExchangeFormat, nullptr, &ResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Create(&client, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr, &rpcExchangeFormat, nullptr, &ResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Create(&client, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", nullptr, nullptr, &ResultHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Client_Create(&client, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, nullptr);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

    ib_Rpc_CallHandle* callHandle;
    ib_ByteVector      data = {0, 0};
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
    ib_Rpc_ExchangeFormat rpcExchangeFormat;
    rpcExchangeFormat.mediaType = "A";
    ib_Rpc_Server* server;

    returnCode = ib_Rpc_Server_Create(nullptr, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_Create(&server, nullptr, "functionName", &rpcExchangeFormat, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_Create(&server, (ib_SimulationParticipant*)&mockSimulationParticipant, nullptr, &rpcExchangeFormat, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_Create(&server, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", nullptr, nullptr, &RpcHandler);
    EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    returnCode = ib_Rpc_Server_Create(&server, (ib_SimulationParticipant*)&mockSimulationParticipant, "functionName", &rpcExchangeFormat, nullptr, nullptr);
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
    ib_Rpc_CallHandle* callHandle;
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

}
