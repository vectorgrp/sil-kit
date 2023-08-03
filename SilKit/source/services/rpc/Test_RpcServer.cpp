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

class Test_RpcServer : public RpcTestBase
{
};

TEST_F(Test_RpcServer, rpc_server_call_response_sends_message_with_timestamp_and_data)
{
    SilKit::Core::Tests::MockTimeProvider fixedTimeProvider;
    fixedTimeProvider.now = std::chrono::nanoseconds{123456};
    ON_CALL(fixedTimeProvider, Now()).WillByDefault(testing::Return(fixedTimeProvider.now));

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
