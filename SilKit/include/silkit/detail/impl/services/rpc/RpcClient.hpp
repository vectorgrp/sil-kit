// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Rpc.h"
#include "silkit/capi/SilKitMacros.h"

#include "silkit/services/rpc/IRpcClient.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Rpc {

class RpcClient : public SilKit::Services::Rpc::IRpcClient
{
    using RpcCallResultHandler = SilKit::Services::Rpc::RpcCallResultHandler;

public:
    inline RpcClient(SilKit_Participant* participant, const std::string& canonicalName,
                     const SilKit::Services::Rpc::RpcSpec& rpcSpec,
                     SilKit::Services::Rpc::RpcCallResultHandler rpcCallResultHandler);

    inline ~RpcClient() override = default;

    inline void Call(SilKit::Util::Span<const uint8_t> data, void* userContext) override;

    inline void CallWithTimeout(SilKit::Util::Span<const uint8_t> data, std::chrono::nanoseconds timeout,
                                void* userContext) override;

    inline void SetCallResultHandler(SilKit::Services::Rpc::RpcCallResultHandler handler) override;

private:
    inline static void SilKitCALL TheRpcCallResultHandler(void* context, SilKit_RpcClient* server,
                                                          const SilKit_RpcCallResultEvent* rpcCallResultEvent);

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Rpc::IRpcClient* controller{nullptr};
        HandlerFunction handler{};
    };

private:
    SilKit_RpcClient* _rpcClient{nullptr};

    std::unique_ptr<HandlerData<RpcCallResultHandler>> _rpcCallResultHandler;
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

RpcClient::RpcClient(SilKit_Participant* participant, const std::string& canonicalName,
                     const SilKit::Services::Rpc::RpcSpec& rpcSpec,
                     SilKit::Services::Rpc::RpcCallResultHandler rpcCallResultHandler)
    : _rpcCallResultHandler{std::make_unique<HandlerData<RpcCallResultHandler>>()}
{
    _rpcCallResultHandler->controller = this;
    _rpcCallResultHandler->handler = std::move(rpcCallResultHandler);

    auto labels = MakeRpcSpecView(rpcSpec);

    SilKit_RpcSpec cRpcSpec;
    SilKit_Struct_Init(SilKit_RpcSpec, cRpcSpec);
    cRpcSpec.functionName = rpcSpec.FunctionName().c_str();
    cRpcSpec.mediaType = rpcSpec.MediaType().c_str();
    cRpcSpec.labelList.numLabels = labels.size();
    cRpcSpec.labelList.labels = labels.data();

    const auto returnCode = SilKit_RpcClient_Create(&_rpcClient, participant, canonicalName.c_str(), &cRpcSpec,
                                                    _rpcCallResultHandler.get(), &TheRpcCallResultHandler);
    ThrowOnError(returnCode);
}

void RpcClient::Call(SilKit::Util::Span<const uint8_t> data, void* userContext)
{
    const auto cData = SilKit::Util::ToSilKitByteVector(data);

    const auto returnCode = SilKit_RpcClient_Call(_rpcClient, &cData, userContext);
    ThrowOnError(returnCode);
}

void RpcClient::CallWithTimeout(SilKit::Util::Span<const uint8_t> data, std::chrono::nanoseconds duration,
                                void* userContext)
{
    const auto cData = SilKit::Util::ToSilKitByteVector(data);

    const auto returnCode = SilKit_RpcClient_CallWithTimeout(_rpcClient, &cData, duration.count(), userContext);
    ThrowOnError(returnCode);
}

void RpcClient::SetCallResultHandler(SilKit::Services::Rpc::RpcCallResultHandler handler)
{
    auto handlerData = std::make_unique<HandlerData<RpcCallResultHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_RpcClient_SetCallResultHandler(_rpcClient, handlerData.get(), &TheRpcCallResultHandler);
    ThrowOnError(returnCode);

    _rpcCallResultHandler = std::move(handlerData);
}

void RpcClient::TheRpcCallResultHandler(void* context, SilKit_RpcClient* server,
                                        const SilKit_RpcCallResultEvent* rpcCallResultEvent)
{
    SILKIT_UNUSED_ARG(server);

    SilKit::Services::Rpc::RpcCallResultEvent event{};
    event.timestamp = std::chrono::nanoseconds{rpcCallResultEvent->timestamp};
    event.userContext = rpcCallResultEvent->userContext;
    event.callStatus = static_cast<SilKit::Services::Rpc::RpcCallStatus>(rpcCallResultEvent->callStatus);
    event.resultData = SilKit::Util::ToSpan(rpcCallResultEvent->resultData);

    const auto handlerData = static_cast<HandlerData<RpcCallResultHandler>*>(context);
    handlerData->handler(handlerData->controller, event);
}

} // namespace Rpc
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
