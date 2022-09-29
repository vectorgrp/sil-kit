#pragma once

#include "silkit/capi/Rpc.h"

#include "silkit/services/rpc/IRpcServer.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"

#include "MakeRpcSpecView.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Rpc {

class RpcServer : public SilKit::Services::Rpc::IRpcServer
{
public:
    RpcServer(SilKit_Participant* participant, const std::string& canonicalName,
              const SilKit::Services::Rpc::RpcSpec& rpcSpec, SilKit::Services::Rpc::RpcCallHandler rpcCallHandler)
    {
        _rpcCallHandler = std::move(rpcCallHandler);

        auto labels = MakeRpcSpecView(rpcSpec);

        SilKit_RpcSpec cRpcSpec;
        SilKit_Struct_Init(SilKit_RpcSpec, cRpcSpec);
        cRpcSpec.functionName = rpcSpec.FunctionName().c_str();
        cRpcSpec.mediaType = rpcSpec.MediaType().c_str();
        cRpcSpec.labelList.numLabels = labels.size();
        cRpcSpec.labelList.labels = labels.data();

        const auto returnCode = SilKit_RpcServer_Create(&_rpcServer, participant, canonicalName.c_str(), &cRpcSpec,
                                                        this, &TheRpcCallHandler);
        ThrowOnError(returnCode);
    }

    ~RpcServer() override = default;

    void SubmitResult(SilKit::Services::Rpc::IRpcCallHandle* callHandle,
                      SilKit::Util::Span<const uint8_t> resultData) override
    {
        const auto cResultData = SilKit::Util::ToSilKitByteVector(resultData);

        const auto returnCode = SilKit_RpcServer_SubmitResult(_rpcServer, CallHandleToC(callHandle), &cResultData);
        ThrowOnError(returnCode);
    }

    void SetCallHandler(SilKit::Services::Rpc::RpcCallHandler handler) override
    {
        _rpcCallHandler = std::move(handler);

        const auto returnCode = SilKit_RpcServer_SetCallHandler(_rpcServer, this, &TheRpcCallHandler);
        ThrowOnError(returnCode);
    }

private:
    static void TheRpcCallHandler(void* context, SilKit_RpcServer* server, const SilKit_RpcCallEvent* rpcCallEvent)
    {
        SILKIT_UNUSED_ARG(server);

        SilKit::Services::Rpc::RpcCallEvent event{};
        event.timestamp = std::chrono::nanoseconds{rpcCallEvent->timestamp};
        event.callHandle = CallHandleFromC(rpcCallEvent->callHandle);
        event.argumentData = SilKit::Util::ToSpan(rpcCallEvent->argumentData);

        const auto rpcServer = static_cast<RpcServer*>(context);
        rpcServer->_rpcCallHandler(rpcServer, event);
    }

    static auto CallHandleFromC(SilKit_RpcCallHandle* rpcCallHandle) -> SilKit::Services::Rpc::IRpcCallHandle*
    {
        return static_cast<SilKit::Services::Rpc::IRpcCallHandle*>(static_cast<void*>(rpcCallHandle));
    }

    static auto CallHandleToC(SilKit::Services::Rpc::IRpcCallHandle* rpcCallHandle) -> SilKit_RpcCallHandle*
    {
        return static_cast<SilKit_RpcCallHandle*>(static_cast<void*>(rpcCallHandle));
    }

private:
    SilKit_RpcServer* _rpcServer{nullptr};

    SilKit::Services::Rpc::RpcCallHandler _rpcCallHandler;
};

} // namespace Rpc
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
