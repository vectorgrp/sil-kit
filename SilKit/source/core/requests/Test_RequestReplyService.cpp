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

#include <chrono>
#include <functional>
#include <set>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "RequestReplyService.hpp"
#include "string_utils_internal.hpp"
#include "Uuid.hpp"
#include "MockParticipant.hpp"
#include "MockServiceEndpoint.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Core::RequestReply;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

auto ARequestReplyCallWith(FunctionType functionType, std::vector<uint8_t> callData)
    -> Matcher<const RequestReplyCall&>
{
    return AllOf(Field(&RequestReplyCall::callData, callData), Field(&RequestReplyCall::functionType, functionType));
}

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const RequestReplyCall&), (override));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const std::string& targetParticipantName, const RequestReplyCallReturn&), (override));

    std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const IServiceEndpoint* /*service*/,
        const std::string& /*msgTypeName*/) override
    {
        return {"P1", "P2"};
    }

    size_t GetNumberOfRemoteReceivers(const IServiceEndpoint* /*service*/, const std::string& /*msgTypeName*/) override
    {
        return 2;
    }

};

class MockParticipantReplies : public IRequestReplyProcedure, public IParticipantReplies
{
public:
    // IRequestReplyProcedure
    MOCK_METHOD(void, ReceiveCall, (IRequestReplyService * requestReplyService, Util::Uuid callUuid, std::vector<uint8_t> callData), (override));
    MOCK_METHOD(void, ReceiveCallReturn, (std::string fromParticipant, Util::Uuid callUuid, std::vector<uint8_t> callReturnData, CallReturnStatus callReturnStatus), (override));
    MOCK_METHOD(void, SetRequestReplyServiceEndpoint, (IServiceEndpoint* requestReplyServiceEndpoint), (override));
    // IParticipantReplies
    MOCK_METHOD(void, CallAfterAllParticipantsReplied, (std::function<void()> completionFunction), (override));
};


class Test_RequestReplyService : public testing::Test
{
protected:
    Test_RequestReplyService()
    {
    }

    MockParticipant _participant;
    MockParticipantReplies _participantReplies;

};

// RCP flow 1/4: Client: Call() sends out RequestReplyCall
TEST_F(Test_RequestReplyService, test_call)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data for validation
    RequestReplyCall reqReplCall{};
    reqReplCall.callData = {1, 2, 3};
    reqReplCall.functionType = FunctionType::ParticipantReplies;

    // Test that SendMsg fires on Call()
    // Matcher needed here to exclude UUID which is returned by Call() itself, thus unknown in EXPECT_CALL.
    EXPECT_CALL(_participant, SendMsg(&reqReplService, ARequestReplyCallWith(reqReplCall.functionType, reqReplCall.callData)));
    reqReplCall.callUuid = reqReplService.Call(reqReplCall.functionType, reqReplCall.callData);

    // Test that a call with FunctionType::Invalid throws
    EXPECT_THROW(reqReplService.Call(FunctionType::Invalid, {}), SilKitError);
}

// RCP flow 2/4: Server: Receive a RequestReplyCall and route to procedure
TEST_F(Test_RequestReplyService, test_functiontype_route_call_to_participantreplies)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data for validation
    RequestReplyCall reqReplCall{};
    reqReplCall.callData = {1, 2, 3};
    reqReplCall.callUuid = {1, 2};
    reqReplCall.functionType = FunctionType::ParticipantReplies;

    // Test that FunctionType::ParticipantReplies is routed to ParticipantReplies
    EXPECT_CALL(_participantReplies, ReceiveCall(&reqReplService, reqReplCall.callUuid, reqReplCall.callData));

    // Initiate receive
    MockServiceEndpoint from{"p1", "n1","c1", 2};
    reqReplService.ReceiveMsg(&from, reqReplCall);

    // Invalid function type should throw
    reqReplCall.functionType = FunctionType::Invalid;
    EXPECT_THROW(reqReplService.ReceiveMsg(&from, reqReplCall), SilKitError);
}

// RCP flow 3/4: Server: Reveice a RequestReplyCall; SubmitCallReturn() sends out RequestReplyCallReturn 
TEST_F(Test_RequestReplyService, test_submitcallreturn)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data for validation
    RequestReplyCall reqReplCall{};
    reqReplCall.callData = {1, 2, 3};
    reqReplCall.callUuid = {1, 2};
    reqReplCall.functionType = FunctionType::ParticipantReplies;

    RequestReplyCallReturn reqReplCallReturn{};
    reqReplCallReturn.callReturnData = {4, 5, 6};
    reqReplCallReturn.callUuid = reqReplCall.callUuid;
    reqReplCallReturn.functionType = FunctionType::ParticipantReplies;
    reqReplCallReturn.callReturnStatus = CallReturnStatus::Success;

    // Receive a call, test reception
    MockServiceEndpoint from{"p1", "n1","c1", 2};
    EXPECT_CALL(_participantReplies, ReceiveCall(&reqReplService, reqReplCall.callUuid, reqReplCall.callData));
    reqReplService.ReceiveMsg(&from, reqReplCall);

    // Test that reception of same UUID throws
    EXPECT_THROW(reqReplService.ReceiveMsg(&from, reqReplCall), SilKitError);

    // Test that SubmitCallReturn with FunctionType::Invalid throws
    EXPECT_THROW(reqReplService.SubmitCallReturn({}, FunctionType::Invalid, {}, CallReturnStatus::Success), SilKitError);

    // Test that SubmitCallReturn with unknown UUID throws
    EXPECT_THROW(reqReplService.SubmitCallReturn({1234, 1234}, FunctionType::ParticipantReplies, {}, CallReturnStatus::Success), SilKitError);

    // Test that SendMsg fires on SubmitCallReturn()
    EXPECT_CALL(_participant,
                SendMsg(&reqReplService, from.GetServiceDescriptor().GetParticipantName(), reqReplCallReturn));
    reqReplService.SubmitCallReturn(reqReplCallReturn.callUuid, reqReplCallReturn.functionType,
                                    reqReplCallReturn.callReturnData, CallReturnStatus::Success);
}

// RCP flow 4/4: Client: Receive a RequestReplyCallReturn and route to procedure
TEST_F(Test_RequestReplyService, test_functiontype_route_callreturn_to_participantreplies)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data for validation
    RequestReplyCallReturn reqReplCallReturn{};
    reqReplCallReturn.callReturnData = {4, 5, 6};
    reqReplCallReturn.callUuid = {4, 5};
    reqReplCallReturn.functionType = FunctionType::ParticipantReplies;
    reqReplCallReturn.callReturnStatus = CallReturnStatus::Success;

    // Test that FunctionType::ParticipantReplies is routed to ParticipantReplies
    EXPECT_CALL(_participantReplies, ReceiveCallReturn("p1", reqReplCallReturn.callUuid, reqReplCallReturn.callReturnData,
                                                       reqReplCallReturn.callReturnStatus));

    // Initiate receive
    MockServiceEndpoint from{"p1", "n1","c1", 2};
    reqReplService.ReceiveMsg(&from, reqReplCallReturn);
}

// Call() for two participants, disconnects should trigger a RequestReplyCallReturn in the procedure
TEST_F(Test_RequestReplyService, test_disconnect_during_call)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data for validation
    RequestReplyCall reqReplCall{};
    reqReplCall.callData = {1, 2, 3};
    reqReplCall.functionType = FunctionType::ParticipantReplies;

    EXPECT_CALL(_participant,
                SendMsg(&reqReplService, ARequestReplyCallWith(reqReplCall.functionType, reqReplCall.callData)));
    // Mock of GetParticipantNamesOfRemoteReceivers returns "P1", "P2"
    reqReplCall.callUuid = reqReplService.Call(reqReplCall.functionType, reqReplCall.callData);

    RequestReplyCallReturn reqReplCallReturn{};
    reqReplCallReturn.callReturnData = {};
    reqReplCallReturn.functionType = FunctionType::ParticipantReplies;
    reqReplCallReturn.callUuid = reqReplCall.callUuid;

    // Removing "P1" will trigger the CallReturn via disconnect
    EXPECT_CALL(_participantReplies,
                ReceiveCallReturn("P1", reqReplCallReturn.callUuid, reqReplCallReturn.callReturnData,
                                                       CallReturnStatus::RecipientDisconnected));
    reqReplService.OnParticpantRemoval("P1");

    // Removing "P2" will trigger the CallReturn via disconnect
    EXPECT_CALL(_participantReplies,
                ReceiveCallReturn("P2" , reqReplCallReturn.callUuid, reqReplCallReturn.callReturnData,
                                                       CallReturnStatus::RecipientDisconnected));
    reqReplService.OnParticpantRemoval("P2");
}

TEST_F(Test_RequestReplyService, test_unknown_function_type)
{
    EXPECT_CALL(_participantReplies, SetRequestReplyServiceEndpoint(_));
    RequestReplyService reqReplService{
        &_participant, "ParticipantA", {{FunctionType::ParticipantReplies, &_participantReplies}}};

    // Reference data with an unknown function type
    RequestReplyCall reqReplCall{};
    reqReplCall.callData = {1, 2, 3};
    reqReplCall.callUuid = {1, 2};
    reqReplCall.functionType = static_cast<FunctionType>(10000);

    RequestReplyCallReturn reqReplCallReturn{};
    reqReplCallReturn.callUuid = reqReplCall.callUuid;
    reqReplCallReturn.functionType = reqReplCall.functionType;
    reqReplCallReturn.callReturnStatus = CallReturnStatus::UnknownFunctionType;

    // Initiate receive
    MockServiceEndpoint from{"p1", "n1","c1", 2};
    // The unknown function type cannot be routed to a procedure, the CallReturn is directly
    // submitted with CallReturnStatus::UnknownFunctionType and empty data
    EXPECT_CALL(_participant,
                SendMsg(&reqReplService, from.GetServiceDescriptor().GetParticipantName(), reqReplCallReturn));
    reqReplService.ReceiveMsg(&from, reqReplCall);

}

} // anonymous namespace for test
