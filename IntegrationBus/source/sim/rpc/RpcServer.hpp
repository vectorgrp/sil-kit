// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/rpc/IRpcServer.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToRpcServer.hpp"
#include "IParticipantInternal.hpp"
#include "RpcServerInternal.hpp"
#include "RpcCallHandle.hpp"

namespace ib {
namespace sim {
namespace rpc {

class RpcServer
    : public IRpcServer
    , public IIbToRpcServer
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    RpcServer(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
              const std::string& functionName, const std::string& mediaType,
              const std::map<std::string, std::string>& labels, RpcCallHandler handler);

    void RegisterServiceDiscovery();

    void SetCallHandler(RpcCallHandler handler) override;

    void SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData) override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

private:
    void AddInternalRpcServer(const std::string& clientUUID, std::string joinedMediaType,
                              const std::map<std::string, std::string>& clientLabels);
    std::string _functionName;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    RpcCallHandler _handler;

    mw::ServiceDescriptor _serviceDescriptor{};
    std::vector<RpcServerInternal*> _internalRpcServers;
    mw::logging::ILogger* _logger;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcServer::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcServer::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace rpc
} // namespace sim
} // namespace ib
