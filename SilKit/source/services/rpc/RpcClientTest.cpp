// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcClient.hpp"


#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RpcTestUtilities.hpp"
#include "MockTimeProvider.hpp"

#include "silkit/util/functional.hpp"

namespace {

using namespace SilKit::Services::Rpc;
using namespace SilKit::Services::Rpc::Tests;

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

    rpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    rpcClient->Call(sampleData);
}

TEST_F(RpcClientTest, rpc_client_call_sends_message_with_current_timestamp_and_data)
{
    SilKit::Core::Tests::MockTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};
    ON_CALL(fixedTimeProvider, Now()).WillByDefault(testing::Return(fixedTimeProvider.now));

    CreateRpcServer();
    IRpcClient* iRpcClient = CreateRpcClient();

    EXPECT_CALL(participant->GetSilKitConnection(), Mock_SendMsg(testing::_, testing::A<FunctionCall>()))
        .WillOnce([this, &fixedTimeProvider](const SilKit::Core::IServiceEndpoint* /*from*/, const FunctionCall& msg) {
            ASSERT_EQ(msg.timestamp, fixedTimeProvider.now);
            ASSERT_EQ(msg.data, sampleData);
        });

    // HACK: Change the time provider for the captured services. Must happen _after_ the RpcServer and RpcClient (and
    //       therefore the RpcServerInternal) have been created.
    participant->GetSilKitConnection().Test_SetTimeProvider(&fixedTimeProvider);

    iRpcClient->Call(sampleData);
}

} // anonymous namespace
