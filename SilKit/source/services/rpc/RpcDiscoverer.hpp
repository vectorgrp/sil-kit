// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IServiceDiscovery.hpp"

#include "silkit/services/rpc/RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class RpcDiscoverer
{
public:
    RpcDiscoverer(Core::Discovery::IServiceDiscovery* serviceDiscovery);

    std::vector<RpcDiscoveryResult> GetMatchingRpcServers(const std::string& functionName, const std::string& mediaType,
                                                          const std::map<std::string, std::string>& labels) const;

private:
    Core::Discovery::IServiceDiscovery* _serviceDiscovery{nullptr};

};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
