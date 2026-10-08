// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/rpc/RpcClient.hpp"


#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "services/rpc/RpcTestUtilities.hpp"
#include "core/mock/participant/MockTimeProvider.hpp"

#include "util/functional.hpp"

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

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(rpcClient), testing::_)).Times(0);
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

// NB: Destroyed after RpcTestBase, because the RpcClient unregisters its timeout handler from it on destruction.
struct TimeProviderHolder
{
    testing::NiceMock<SilKit::Core::Tests::MockTimeProvider> timeProvider;
};

class Test_RpcClientTimeout
    : public TimeProviderHolder
    , public RpcTestBase
{
protected:
    void AdvanceTime(std::chrono::nanoseconds duration)
    {
        timeProvider.now += duration;
        timeProvider._handlers.InvokeAll(timeProvider.now, duration);
    }
};

TEST_F(Test_RpcClientTimeout, answered_call_does_not_time_out)
{
    using namespace std::chrono_literals;

    // A server without call handler answers immediately with an internal server error
    CreateRpcServer();
    IRpcClient* iRpcClient = CreateRpcClient();
    iRpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    participant->GetSilKitConnection().Test_SetTimeProvider(&timeProvider);

    EXPECT_CALL(callbacks, CallResultHandler(testing::Eq(iRpcClient),
                                             testing::Field(&RpcCallResultEvent::callStatus,
                                                            RpcCallStatus::InternalServerError)))
        .Times(1);

    iRpcClient->CallWithTimeout(sampleData, 10ms);
    AdvanceTime(10ms);
    AdvanceTime(10ms);
}

TEST_F(Test_RpcClientTimeout, call_times_out_once_and_late_reply_is_ignored)
{
    using namespace std::chrono_literals;

    IRpcServer* iRpcServer = CreateRpcServer();
    IRpcCallHandle* callHandle{nullptr};
    iRpcServer->SetCallHandler([&callHandle](IRpcServer*, const RpcCallEvent& event) { callHandle = event.callHandle; });

    IRpcClient* iRpcClient = CreateRpcClient();
    iRpcClient->SetCallResultHandler(SilKit::Util::bind_method(&callbacks, &Callbacks::CallResultHandler));
    participant->GetSilKitConnection().Test_SetTimeProvider(&timeProvider);

    const auto userContext = reinterpret_cast<void*>(uintptr_t(12345));
    EXPECT_CALL(callbacks, CallResultHandler(
                               testing::Eq(iRpcClient),
                               testing::AllOf(testing::Field(&RpcCallResultEvent::callStatus, RpcCallStatus::Timeout),
                                              testing::Field(&RpcCallResultEvent::userContext, userContext))))
        .Times(1);

    iRpcClient->CallWithTimeout(sampleData, 10ms, userContext);
    ASSERT_NE(callHandle, nullptr);

    AdvanceTime(5ms);
    AdvanceTime(5ms);
    AdvanceTime(5ms);

    iRpcServer->SubmitResult(callHandle, sampleData);
}

} // anonymous namespace
