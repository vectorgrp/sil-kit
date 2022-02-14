// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IServiceDiscovery.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

class RpcDiscoverer
{
public:
    RpcDiscoverer(mw::service::IServiceDiscovery* serviceDiscovery);

    std::vector<RpcDiscoveryResult> GetMatchingRpcServers(
        const std::string& functionName, const RpcExchangeFormat& exchangeFormat,
        const std::map<std::string, std::string>& labels) const;

private:
    mw::service::IServiceDiscovery* _serviceDiscovery{nullptr};

};

} // namespace rpc
} // namespace sim
} // namespace ib
