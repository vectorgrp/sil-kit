// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/rpc/IRpcServer.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToRpcServer.hpp"
#include "IComAdapterInternal.hpp"
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
    RpcServer(mw::IComAdapterInternal* comAdapter, cfg::RpcPort config,
        mw::sync::ITimeProvider* timeProvider, CallProcessor handler);

    void RegisterServiceDiscovery();

    void SetRpcHandler(CallProcessor handler) override;

    auto Config() const -> const cfg::RpcPort& override;

    void SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData) override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

private:

    void AddInternalRpcServer(const std::string& clientUUID, RpcExchangeFormat joinedExchangeFormat,
                              const std::map<std::string, std::string>& clientLabels);

    cfg::RpcPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    std::vector<RpcServerInternal*> _internalRpcServers;
    CallProcessor _handler;
    mw::logging::ILogger* _logger;

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
