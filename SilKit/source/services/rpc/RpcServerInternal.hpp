// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
