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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ByteVectorMatcher.hpp"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/util/Span.hpp"

#include "MockCapiTest.hpp"

#include <algorithm>

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::StrEq;
using testing::Return;

using namespace SilKit::Services::Rpc;
using SilKit::Services::MatchingLabel;
using SilKit::Util::Span;

struct MatchingLabelEquals
{
    bool operator()(const MatchingLabel& lhs, const MatchingLabel& rhs) const
    {
        return std::make_tuple(lhs.key, lhs.value, lhs.kind) == std::make_tuple(rhs.key, rhs.value, rhs.kind);
    }
};

struct MatchingLabelLess
{
    bool operator()(const MatchingLabel& lhs, const MatchingLabel& rhs) const
    {
        return std::less<>{}(std::make_tuple(lhs.key, lhs.value, lhs.kind),
                             std::make_tuple(rhs.key, rhs.value, rhs.kind));
    }
};

MATCHER_P(RpcSpecMatcher, rpcSpecParam, "")
{
    const RpcSpec& cppRpcSpec = rpcSpecParam;
    const SilKit_RpcSpec* cRpcSpec = arg;

    if (cRpcSpec == nullptr)
    {
        *result_listener << "cRpcSpec must not be nullptr";
        return false;
    }

    if (cRpcSpec->functionName == nullptr)
    {
        *result_listener << "cRpcSpec->functionName must not be nullptr";
        return false;
    }

    if (cRpcSpec->mediaType == nullptr)
    {
        *result_listener << "cRpcSpec->mediaType must not be nullptr";
        return false;
    }

    if (cppRpcSpec.FunctionName() != cRpcSpec->functionName)
    {
        *result_listener << "functionName does not match";
        return false;
    }

    if (cppRpcSpec.MediaType() != cRpcSpec->mediaType)
    {
        *result_listener << "mediaType does not match";
        return false;
    }

    if (cRpcSpec->labelList.numLabels != cppRpcSpec.Labels().size())
    {
        *result_listener << "number-of-labels does not match";
        return false;
    }

    using MatchingLabelSet = std::set<MatchingLabel, MatchingLabelLess>;

    MatchingLabelSet cppLabels{cppRpcSpec.Labels().begin(), cppRpcSpec.Labels().end()};

    MatchingLabelSet cLabels{};
    std::transform(cRpcSpec->labelList.labels, cRpcSpec->labelList.labels + cRpcSpec->labelList.numLabels,
                   std::inserter(cLabels, cLabels.begin()), [](const SilKit_Label& cLabel) {
                       return MatchingLabel{cLabel.key, cLabel.value, static_cast<MatchingLabel::Kind>(cLabel.kind)};
                   });

    if (!std::equal(cppLabels.begin(), cppLabels.end(), cLabels.begin(), cLabels.end(), MatchingLabelEquals{}))
    {
        *result_listener << "labels do not match";
        return false;
    }

    return true;
}

class Test_HourglassRpc : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_RpcServer* mockRpcServer{reinterpret_cast<SilKit_RpcServer*>(uintptr_t(0x78563412))};
    SilKit_RpcClient* mockRpcClient{reinterpret_cast<SilKit_RpcClient*>(uintptr_t(0x87654321))};

    Test_HourglassRpc()
    {
        using testing::_;
        ON_CALL(capi, SilKit_RpcServer_Create(_, _, _, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockRpcServer), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_RpcClient_Create(_, _, _, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockRpcClient), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

// RpcServer

TEST_F(Test_HourglassRpc, SilKit_RpcServer_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string serverName = "RpcServer1";
    std::string functionName = "RpcFunctionName1";
    std::string mediaType = "RpcMediaType1";
    std::string labelKey1 = "LabelKey1";
    std::string labelKey2 = "LabelKey2";
    std::string labelValue1 = "LabelValue1";
    std::string labelValue2 = "LabelValue2";

    RpcSpec rpcSpec{functionName, mediaType};
    rpcSpec.AddLabel(labelKey1, labelValue1, MatchingLabel::Kind::Mandatory);
    rpcSpec.AddLabel(labelKey2, labelValue2, MatchingLabel::Kind::Optional);

    EXPECT_CALL(capi, SilKit_RpcServer_Create(testing::_, participant, StrEq(serverName), RpcSpecMatcher(rpcSpec),
                                              testing::_, testing::_));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcServer rpcServer{
        participant, serverName, rpcSpec, [](IRpcServer*, const RpcCallEvent&) {
            // do nothing
        }};
}

TEST_F(Test_HourglassRpc, SilKit_RpcServer_SubmitResult)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcServer rpcServer{
        participant, "RpcServer1", RpcSpec{"FunctionName1", "MediaType1"}, [](IRpcServer*, const RpcCallEvent&) {
            // do nothing
        }};

    auto* const rpcCallHandle = reinterpret_cast<SilKit_RpcCallHandle*>(uintptr_t(654321));

    std::vector<uint8_t> bytes{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const Span<uint8_t> byteSpan{bytes};

    EXPECT_CALL(capi, SilKit_RpcServer_SubmitResult(mockRpcServer, rpcCallHandle, ByteVectorMatcher(byteSpan)));

    rpcServer.SubmitResult(reinterpret_cast<IRpcCallHandle*>(rpcCallHandle), byteSpan);
}

TEST_F(Test_HourglassRpc, SilKit_RpcServer_SetCallHandler)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcServer rpcServer{
        participant, "RpcServer1", RpcSpec{"FunctionName1", "MediaType1"}, [](IRpcServer*, const RpcCallEvent&) {
            // do nothing
        }};

    EXPECT_CALL(capi, SilKit_RpcServer_SetCallHandler(mockRpcServer, testing::_, testing::_));

    rpcServer.SetCallHandler([](IRpcServer*, const RpcCallEvent&) {
        // do nothing
    });
}

// RpcClient

TEST_F(Test_HourglassRpc, SilKit_RpcClient_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string clientName = "RpcClient1";
    std::string functionName = "RpcFunctionName1";
    std::string mediaType = "RpcMediaType1";
    std::string labelKey1 = "LabelKey1";
    std::string labelKey2 = "LabelKey2";
    std::string labelValue1 = "LabelValue1";
    std::string labelValue2 = "LabelValue2";

    RpcSpec rpcSpec{functionName, mediaType};
    rpcSpec.AddLabel(labelKey1, labelValue1, MatchingLabel::Kind::Mandatory);
    rpcSpec.AddLabel(labelKey2, labelValue2, MatchingLabel::Kind::Optional);

    EXPECT_CALL(capi, SilKit_RpcClient_Create(testing::_, participant, StrEq(clientName), RpcSpecMatcher(rpcSpec),
                                              testing::_, testing::_));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcClient rpcClient{
        participant, clientName, rpcSpec, [](IRpcClient*, const RpcCallResultEvent&) {
            // do nothing
        }};
}

TEST_F(Test_HourglassRpc, SilKit_RpcClient_Call)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcClient rpcClient{
        participant, "RpcClient1", RpcSpec{"FunctionName1", "MediaType1"}, [](IRpcClient*, const RpcCallResultEvent&) {
            // do nothing
        }};

    std::vector<uint8_t> bytes{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const Span<uint8_t> byteSpan{bytes};

    EXPECT_CALL(capi, SilKit_RpcClient_Call(mockRpcClient, ByteVectorMatcher(byteSpan), testing::_));

    rpcClient.Call(byteSpan, nullptr);
}

TEST_F(Test_HourglassRpc, SilKit_RpcClient_SetCallResultHandler)
{
    auto* const participant = reinterpret_cast<SilKit_Participant*>(uintptr_t(123456));

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Rpc::RpcClient rpcClient{
        participant, "RpcClient1", RpcSpec{"FunctionName1", "MediaType1"}, [](IRpcClient*, const RpcCallResultEvent&) {
            // do nothing
        }};

    EXPECT_CALL(capi, SilKit_RpcClient_SetCallResultHandler(mockRpcClient, testing::_, testing::_));

    rpcClient.SetCallResultHandler([](IRpcClient*, const RpcCallResultEvent&) {
        // do nothing
    });
}

} //namespace
