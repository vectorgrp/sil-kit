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

#include "RpcClient.hpp"


#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RpcTestUtilities.hpp"
#include "MockTimeProvider.hpp"

#include "functional.hpp"

namespace {

using namespace SilKit::Services::Rpc;
using namespace SilKit::Services::Rpc::Tests;

class Test_RpcClient : public RpcTestBase
{
};

TEST_F(Test_RpcClient, rpc_client_calls_result_handler_with_error_when_no_server_available)
{
    IRpcClient* rpcClient = CreateRpcClient();

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(rpcClient), testing::_))
        .WillOnce([](IRpcClient* /*rpcClient*/, const RpcCallResultEvent& event) {
            ASSERT_EQ(event.callStatus, RpcCallStatus::ServerNotReachable);
        });

    rpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    rpcClient->Call(sampleData);
}

TEST_F(Test_RpcClient, rpc_client_does_not_fail_on_timeout_reply)
{
    // Rpc client should ignore unknown (late timout) call results

    IRpcClient* rpcClient = CreateRpcClient();
    rpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    rpcClient->Call(sampleData);

    RpcClient* rpcClientInternal = dynamic_cast<RpcClient*>(rpcClient);

    const FunctionCallResponse response{};
    EXPECT_NO_THROW(rpcClientInternal->ReceiveMessage(response));

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(rpcClient), testing::_))
        .Times(0);
}

TEST_F(Test_RpcClient, rpc_client_call_sends_message_with_current_timestamp_and_data)
{
    SilKit::Core::Tests::MockTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};
    ON_CALL(fixedTimeProvider, Now()).WillByDefault(testing::Return(fixedTimeProvider.now));

    participant->GetSilKitConnection().Test_SetTimeProvider(&fixedTimeProvider);
    IRpcServer* iRpcServer = CreateRpcServer();
    iRpcServer->SetCallHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallHandler));

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

TEST_F(Test_RpcClient, rpc_client_call_receives_internal_server_error_when_server_has_no_handler)
{
    SilKit::Core::Tests::MockTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};
    ON_CALL(fixedTimeProvider, Now()).WillByDefault(testing::Return(fixedTimeProvider.now));

    CreateRpcServer();
    IRpcClient* iRpcClient = CreateRpcClient();
    iRpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));

    const auto userContext = reinterpret_cast<void*>(uintptr_t(12345));

    const auto rpcCallResultEventMatcher = testing::Matcher<RpcCallResultEvent>{
        testing::AllOf(testing::Field(&RpcCallResultEvent::timestamp, fixedTimeProvider.now),
                       testing::Field(&RpcCallResultEvent::userContext, userContext),
                       testing::Field(&RpcCallResultEvent::resultData, testing::IsEmpty()),
                       testing::Field(&RpcCallResultEvent::callStatus, RpcCallStatus::InternalServerError))};

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(iRpcClient), rpcCallResultEventMatcher)).Times(1);

    // HACK: Change the time provider for the captured services. Must happen _after_ the RpcServer and RpcClient (and
    //       therefore the RpcServerInternal) have been created.
    participant->GetSilKitConnection().Test_SetTimeProvider(&fixedTimeProvider);

    iRpcClient->Call(sampleData, userContext);
}

} // anonymous namespace
