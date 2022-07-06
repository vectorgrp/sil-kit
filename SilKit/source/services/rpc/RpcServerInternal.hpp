// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <map>

#include "silkit/core/fwd_decl.hpp"
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
    , public Core::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    RpcServerInternal(Core::IParticipantInternal* participant, Core::Orchestration::ITimeProvider* timeProvider,
                      const std::string& functionName, const std::string& mediaType,
                      const std::map<std::string, std::string>& labels, const std::string& clientUUID,
                      SilKit::Services::Rpc::RpcCallHandler handler, IRpcServer* parent);

    void SetRpcHandler(RpcCallHandler handler);

    void SubmitResult(IRpcCallHandle* callHandlePtr, const std::vector<uint8_t>& resultData);

    //! \brief Accepts messages originating from SilKit communications.
    void ReceiveSilKitMessage(const Core::IServiceEndpoint* from, const FunctionCall& msg) override;
    void ReceiveMessage(const FunctionCall& msg);

    // SilKit::Core::Orchestration::ITimeConsumer
    void SetTimeProvider(Core::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

private:
    std::string _functionName;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;
    RpcCallHandler _handler;
    IRpcServer* _parent;

    Core::ServiceDescriptor _serviceDescriptor{};
    std::map<std::string, std::unique_ptr<CallHandleImpl>> _receivedCallHandles;
    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
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
