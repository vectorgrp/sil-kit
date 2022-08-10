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

#include "RpcServerInternal.hpp"

#include "silkit/services/rpc/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "RpcDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcServerInternal::RpcServerInternal(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                                     const std::string& functionName, const std::string& mediaType,
                                     const std::vector<SilKit::Services::MatchingLabel>& labels,
                                     const std::string& clientUUID,
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
    if (!_handler)
    {
        // Inform the client about the failed (unhandled) call
        _participant->SendMsg(
            this,
            FunctionCallResponse{_timeProvider->Now(), msg.callUuid, {}, FunctionCallResponse::Status::InternalError});

        // Log that a call was received that could not be handled
        _participant->GetLogger()->Error("RpcServerInternal: FunctionCall received but no handler has been set");

        return;
    }

    // NB: 'result' has type pair<iterator, bool> where the bool indicates if the call was actually inserted (i.e.
    //     the key was _not_ already present in the map).
    auto result = _activeCalls.emplace(msg.callUuid, std::make_shared<RpcCallHandle>(msg.callUuid));
    if (!result.second)
    {
        // Inform the client about the failed (unhandled) call
        _participant->SendMsg(
            this,
            FunctionCallResponse{_timeProvider->Now(), msg.callUuid, {}, FunctionCallResponse::Status::InternalError});

        // Log that a call was received that could not be handled
        _participant->GetLogger()->Error("RpcServerInternal: Received FunctionCall with already active callUuid");

        return;
    }

    // NB: Explicitly _copy_ the call handle to keep the handle itself alive even if it gets removed from the map
    //     due to a call to SubmitResult in the handler.
    std::shared_ptr<RpcCallHandle> callHandle = result.first->second;
    _handler(_parent, RpcCallEvent{msg.timestamp, callHandle.get(), msg.data});
}

bool RpcServerInternal::SubmitResult(IRpcCallHandle* callHandlePtr, Util::Span<const uint8_t> resultData)
{
    const auto& callHandle = static_cast<const RpcCallHandle&>(*callHandlePtr);

    auto it = _activeCalls.find(callHandle.GetCallUuid());
    if (it == _activeCalls.end())
    {
        // The call is not known to this RpcServerInternal, therefore return false
        return false;
    }

    _participant->SendMsg(
        this, FunctionCallResponse{_timeProvider->Now(), callHandle.GetCallUuid(), Util::ToStdVector(resultData),
                                   FunctionCallResponse::Status::Success});
    _activeCalls.erase(it);

    // The call was handled, therefore return true
    return true;
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
