// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcClient.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RpcTestUtilities.hpp"

#include "silkit/util/functional.hpp"

namespace {

using namespace SilKit::Services::Rpc;
using namespace SilKit::Services::Rpc::Tests;

class RpcServerTest : public RpcTestBase
{
};

TEST_F(RpcServerTest, rpc_server_call_response_sends_message_with_timestamp_and_data)
{
    FixedTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};

    IRpcServer* iRpcServer = CreateRpcServer();

    // Simple call handler that just returns the argument data
    iRpcServer->SetCallHandler([](IRpcServer* iRpcServer, RpcCallEvent event) {
        iRpcServer->SubmitResult(event.callHandle, std::move(event.argumentData));
    });

    EXPECT_CALL(participant->GetSilKitConnection(), Mock_SendMsg(testing::_, testing::A<FunctionCallResponse>()))
        .WillOnce(
            [this, &fixedTimeProvider](const SilKit::Core::IServiceEndpoint* /*from*/, const FunctionCallResponse& msg) {
                ASSERT_EQ(msg.timestamp, fixedTimeProvider.now);
                ASSERT_EQ(msg.data, sampleData);
            });

    IRpcClient* iRpcClient = CreateRpcClient();

    // HACK: Change the time provider for the captured services, must happen _after_ the RpcServer and RpcClient (and
    //       therefore the RpcServerInternal) have been created.
    participant->GetSilKitConnection().Test_SetTimeProvider(&fixedTimeProvider);

    iRpcClient->Call(sampleData);
}

} // anonymous namespace
