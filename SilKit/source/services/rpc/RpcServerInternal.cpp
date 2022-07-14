// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcServerInternal.hpp"
#include "RpcDatatypeUtils.hpp"
#include "silkit/services/rpc/string_utils.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcServerInternal::RpcServerInternal(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                                     const std::string& functionName, const std::string& mediaType,
                                     const std::map<std::string, std::string>& labels, const std::string& clientUUID,
                                     RpcCallHandler handler, IRpcServer* parent)
    : _functionName{functionName}
    , _mediaType{mediaType}
    , _labels{labels}
    , _clientUUID{clientUUID}
    , _handler{std::move(handler)}
    , _parent{parent}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void RpcServerInternal::ReceiveMsg(const Core::IServiceEndpoint* /*from*/, const FunctionCall& msg)
{
    ReceiveMessage(msg);
}

void RpcServerInternal::ReceiveMessage(const FunctionCall& msg)
{
    if (_handler)
    {
        // NB: We keep the ownership to keep the passed IRpcCallHandle* alive
        auto callHandle = std::make_unique<CallHandleImpl>(msg.callUUID);
        auto callHandlePtr = callHandle.get();
        _receivedCallHandles[to_string(msg.callUUID)] = std::move(callHandle);
        _handler(_parent, RpcCallEvent{msg.timestamp, callHandlePtr, msg.data});
    }
}

void RpcServerInternal::SubmitResult(IRpcCallHandle* callHandlePtr, Util::Span<const uint8_t> resultData)
{
    auto callHandle = static_cast<const CallHandleImpl&>(*callHandlePtr);
    auto callHandleStr = to_string(callHandle._callUUID);
    auto it = _receivedCallHandles.find(callHandleStr);
    if (it != _receivedCallHandles.end())
    {
        _participant->SendMsg(this, FunctionCallResponse{_timeProvider->Now(), callHandle._callUUID, Util::ToStdVector(resultData)});
        _receivedCallHandles.erase(callHandleStr);
    }
}

void RpcServerInternal::SetRpcHandler(RpcCallHandler handler)
{
    _handler = std::move(handler);
}

void RpcServerInternal::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
