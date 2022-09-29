#pragma once

#include "silkit/capi/Rpc.h"

#include "silkit/services/rpc/IRpcClient.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"

#include "MakeRpcSpecView.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Rpc {

class RpcClient : public SilKit::Services::Rpc::IRpcClient
{
public:
    RpcClient(SilKit_Participant* participant, const std::string& canonicalName,
              const SilKit::Services::Rpc::RpcSpec& rpcSpec,
              SilKit::Services::Rpc::RpcCallResultHandler rpcCallResultHandler)
    {
        _rpcCallResultHandler = std::move(rpcCallResultHandler);

        auto labels = MakeRpcSpecView(rpcSpec);

        SilKit_RpcSpec cRpcSpec;
        SilKit_Struct_Init(SilKit_RpcSpec, cRpcSpec);
        cRpcSpec.functionName = rpcSpec.FunctionName().c_str();
        cRpcSpec.mediaType = rpcSpec.MediaType().c_str();
        cRpcSpec.labelList.numLabels = labels.size();
        cRpcSpec.labelList.labels = labels.data();

        const auto returnCode = SilKit_RpcClient_Create(&_rpcClient, participant, canonicalName.c_str(), &cRpcSpec,
                                                        this, &TheRpcCallResultHandler);
        ThrowOnError(returnCode);
    }

    ~RpcClient() override = default;

    void Call(SilKit::Util::Span<const uint8_t> data, void* userContext) override
    {
        const auto cData = SilKit::Util::ToSilKitByteVector(data);

        const auto returCode = SilKit_RpcClient_Call(_rpcClient, &cData, userContext);
        ThrowOnError(returCode);
    }

    void SetCallResultHandler(SilKit::Services::Rpc::RpcCallResultHandler handler) override
    {
        _rpcCallResultHandler = std::move(handler);

        const auto returnCode = SilKit_RpcClient_SetCallResultHandler(_rpcClient, this, &TheRpcCallResultHandler);
        ThrowOnError(returnCode);
    }

private:
    static void TheRpcCallResultHandler(void* context, SilKit_RpcClient* server,
                                        const SilKit_RpcCallResultEvent* rpcCallResultEvent)
    {
        SILKIT_UNUSED_ARG(server);

        SilKit::Services::Rpc::RpcCallResultEvent event{};
        event.timestamp = std::chrono::nanoseconds{rpcCallResultEvent->timestamp};
        event.userContext = rpcCallResultEvent->userContext;
        event.callStatus = static_cast<SilKit::Services::Rpc::RpcCallStatus>(rpcCallResultEvent->callStatus);
        event.resultData = SilKit::Util::ToSpan(rpcCallResultEvent->resultData);

        const auto rpcClient = static_cast<RpcClient*>(context);
        rpcClient->_rpcCallResultHandler(rpcClient, event);
    }

private:
    SilKit_RpcClient* _rpcClient{nullptr};

    SilKit::Services::Rpc::RpcCallResultHandler _rpcCallResultHandler;
};

} // namespace Rpc
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
