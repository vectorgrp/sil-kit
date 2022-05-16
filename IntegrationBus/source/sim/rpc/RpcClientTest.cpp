// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcClient.hpp"

#include "RpcTestUtilities.hpp"

#include <ib/util/functional.hpp>

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace {

using namespace ib::sim::rpc;
using namespace ib::sim::rpc::test;

class RpcClientTest : public RpcTestBase
{
};

TEST_F(RpcClientTest, rpc_client_calls_result_handler_with_error_when_no_server_available)
{
    IRpcClient* rpcClient = CreateRpcClient();

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(rpcClient), testing::_))
        .WillOnce([](IRpcClient* /*rpcClient*/, const RpcCallResultEvent& event) {
            ASSERT_EQ(event.callStatus, RpcCallStatus::ServerNotReachable);
        });

    rpcClient->SetCallResultHandler(ib::util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    rpcClient->Call(sampleData);
}

TEST_F(RpcClientTest, rpc_client_call_sends_message_with_current_timestamp_and_data)
{
    FixedTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};

    CreateRpcServer();
    IRpcClient* iRpcClient = CreateRpcClient();

    EXPECT_CALL(participant->GetIbConnection(), Mock_SendIbMessage(testing::_, testing::A<FunctionCall>()))
        .WillOnce([this, &fixedTimeProvider](const ib::mw::IIbServiceEndpoint* /*from*/, const FunctionCall& msg) {
            ASSERT_EQ(msg.timestamp, fixedTimeProvider.now);
            ASSERT_EQ(msg.data, sampleData);
        });

    // HACK: Change the time provider for the captured services. Must happen _after_ the RpcServer and RpcClient (and
    //       therefore the RpcServerInternal) have been created.
    participant->GetIbConnection().Test_SetTimeProvider(&fixedTimeProvider);

    iRpcClient->Call(sampleData);
}

} // anonymous namespace
