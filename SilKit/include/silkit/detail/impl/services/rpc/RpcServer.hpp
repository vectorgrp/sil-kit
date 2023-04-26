// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "silkit/capi/Rpc.h"

#include "silkit/services/rpc/IRpcServer.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Rpc {

class RpcServer : public SilKit::Services::Rpc::IRpcServer
{
    using RpcCallHandler = SilKit::Services::Rpc::RpcCallHandler;

public:
    inline RpcServer(SilKit_Participant* participant, const std::string& canonicalName,
                     const SilKit::Services::Rpc::RpcSpec& rpcSpec,
                     SilKit::Services::Rpc::RpcCallHandler rpcCallHandler);

    inline ~RpcServer() override = default;

    inline void SubmitResult(SilKit::Services::Rpc::IRpcCallHandle* callHandle,
                             SilKit::Util::Span<const uint8_t> resultData) override;

    inline void SetCallHandler(SilKit::Services::Rpc::RpcCallHandler handler) override;

private:
    inline static void TheRpcCallHandler(void* context, SilKit_RpcServer* server,
                                         const SilKit_RpcCallEvent* rpcCallEvent);

    inline static auto CallHandleFromC(SilKit_RpcCallHandle* rpcCallHandle) -> SilKit::Services::Rpc::IRpcCallHandle*;

    inline static auto CallHandleToC(SilKit::Services::Rpc::IRpcCallHandle* rpcCallHandle) -> SilKit_RpcCallHandle*;

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Rpc::IRpcServer* controller{nullptr};
        HandlerFunction handler{};
    };

private:
    SilKit_RpcServer* _rpcServer{nullptr};

    std::unique_ptr<HandlerData<RpcCallHandler>> _rpcCallHandler;
};

} // namespace Rpc
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MakeRpcSpecView.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Rpc {

RpcServer::RpcServer(SilKit_Participant* participant, const std::string& canonicalName,
                     const SilKit::Services::Rpc::RpcSpec& rpcSpec,
                     SilKit::Services::Rpc::RpcCallHandler rpcCallHandler)
    : _rpcCallHandler{std::make_unique<HandlerData<RpcCallHandler>>()}
{
    _rpcCallHandler->controller = this;
    _rpcCallHandler->handler = std::move(rpcCallHandler);

    auto labels = MakeRpcSpecView(rpcSpec);

    SilKit_RpcSpec cRpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, cRpcSpec);
    cRpcSpec.functionName = rpcSpec.FunctionName().c_str();
    cRpcSpec.mediaType = rpcSpec.MediaType().c_str();
    cRpcSpec.labelList.numLabels = labels.size();
    cRpcSpec.labelList.labels = labels.data();

    const auto returnCode = SilKit_RpcServer_Create(&_rpcServer, participant, canonicalName.c_str(), &cRpcSpec,
                                                    _rpcCallHandler.get(), &TheRpcCallHandler);
    ThrowOnError(returnCode);
}

void RpcServer::SubmitResult(SilKit::Services::Rpc::IRpcCallHandle* callHandle,
                             SilKit::Util::Span<const uint8_t> resultData)
{
    const auto cResultData = SilKit::Util::ToSilKitByteVector(resultData);

    const auto returnCode = SilKit_RpcServer_SubmitResult(_rpcServer, CallHandleToC(callHandle), &cResultData);
    ThrowOnError(returnCode);
}

void RpcServer::SetCallHandler(SilKit::Services::Rpc::RpcCallHandler handler)
{
    auto handlerData = std::make_unique<HandlerData<RpcCallHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_RpcServer_SetCallHandler(_rpcServer, this, &TheRpcCallHandler);
    ThrowOnError(returnCode);

    _rpcCallHandler = std::move(handlerData);
}

void RpcServer::TheRpcCallHandler(void* context, SilKit_RpcServer* server, const SilKit_RpcCallEvent* rpcCallEvent)
{
    SILKIT_UNUSED_ARG(server);

    SilKit::Services::Rpc::RpcCallEvent event{};
    event.timestamp = std::chrono::nanoseconds{rpcCallEvent->timestamp};
    event.callHandle = CallHandleFromC(rpcCallEvent->callHandle);
    event.argumentData = SilKit::Util::ToSpan(rpcCallEvent->argumentData);

    const auto handlerData = static_cast<HandlerData<RpcCallHandler>*>(context);
    handlerData->handler(handlerData->controller, event);
}

auto RpcServer::CallHandleFromC(SilKit_RpcCallHandle* rpcCallHandle) -> SilKit::Services::Rpc::IRpcCallHandle*
{
    return static_cast<SilKit::Services::Rpc::IRpcCallHandle*>(static_cast<void*>(rpcCallHandle));
}

auto RpcServer::CallHandleToC(SilKit::Services::Rpc::IRpcCallHandle* rpcCallHandle) -> SilKit_RpcCallHandle*
{
    return static_cast<SilKit_RpcCallHandle*>(static_cast<void*>(rpcCallHandle));
}

} // namespace Rpc
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
