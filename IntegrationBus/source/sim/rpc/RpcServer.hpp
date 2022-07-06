// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "silkit/core/fwd_decl.hpp"
#include "silkit/services/rpc/IRpcServer.hpp"
#include "silkit/services/rpc/IRpcCallHandle.hpp"

#include "ITimeConsumer.hpp"
#include "IMsgForRpcServer.hpp"
#include "IParticipantInternal.hpp"
#include "RpcServerInternal.hpp"
#include "RpcCallHandle.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class RpcServer
    : public IRpcServer
    , public IMsgForRpcServer
    , public Core::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    RpcServer(Core::IParticipantInternal* participant, Core::Orchestration::ITimeProvider* timeProvider,
              const std::string& functionName, const std::string& mediaType,
              const std::map<std::string, std::string>& labels, RpcCallHandler handler);

    void RegisterServiceDiscovery();

    void SetCallHandler(RpcCallHandler handler) override;

    void SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData) override;

    //SilKit::Core::Orchestration::ITimeConsumer
    void SetTimeProvider(Core::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

private:
    void AddInternalRpcServer(const std::string& clientUUID, std::string joinedMediaType,
                              const std::map<std::string, std::string>& clientLabels);
    std::string _functionName;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    RpcCallHandler _handler;

    Core::ServiceDescriptor _serviceDescriptor{};
    std::vector<RpcServerInternal*> _internalRpcServers;
    Core::Logging::ILogger* _logger;
    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcServer::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcServer::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
