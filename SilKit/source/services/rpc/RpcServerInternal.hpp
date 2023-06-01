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

#pragma once

#include <vector>
#include <map>

#include "ITimeConsumer.hpp"
#include "silkit/services/rpc/IRpcServer.hpp"
#include "silkit/services/rpc/IRpcCallHandle.hpp"

#include "IParticipantInternal.hpp"
#include "IMsgForRpcServerInternal.hpp"
#include "RpcCallHandle.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class RpcServerInternal
    : public IMsgForRpcServerInternal
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    RpcServerInternal(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                      const std::string& functionName, const std::string& mediaType,
                      const std::vector<SilKit::Services::MatchingLabel>& labels, const std::string& clientUUID,
                      SilKit::Services::Rpc::RpcCallHandler handler, IRpcServer* parent);

    void SetRpcHandler(RpcCallHandler handler);

    //! \brief Tries to submit the result to the call associated with the call handle.
    //! \param callHandlePtr The call handle identifying the call to submit a result for
    //! \param resultData The result of the call
    //! \returns True if the call was handled, false if the call was unknown to this RpcServerInternal
    bool SubmitResult(IRpcCallHandle* callHandlePtr, Util::Span<const uint8_t> resultData);

    //! \brief Accepts messages originating from SIL Kit communications.
    void ReceiveMsg(const Core::IServiceEndpoint* from, const FunctionCall& msg) override;
    void ReceiveMessage(const FunctionCall& msg);

    // SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

private:
    std::string _functionName;
    std::string _mediaType;
    std::vector<SilKit::Services::MatchingLabel> _labels;
    std::string _clientUUID;
    RpcCallHandler _handler;
    IRpcServer* _parent;

    Core::ServiceDescriptor _serviceDescriptor{};
    std::map<Util::Uuid, std::shared_ptr<RpcCallHandle>> _activeCalls;
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcServerInternal::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcServerInternal::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
