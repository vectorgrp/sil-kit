// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcDiscoverer.hpp"
#include "RpcDatatypeUtils.hpp"
#include "YamlParser.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcDiscoverer::RpcDiscoverer(Core::Discovery::IServiceDiscovery* serviceDiscovery) : _serviceDiscovery { serviceDiscovery }
{
}

std::vector<RpcDiscoveryResult> RpcDiscoverer::GetMatchingRpcServers(
    const std::string& functionName, const std::string& mediaType, const std::map<std::string, std::string>& labels) const
{
    std::vector<RpcDiscoveryResult> discoveryResults;
    auto serviceDescriptors = _serviceDiscovery->GetServices();

    for (const auto& serviceDescriptor : serviceDescriptors)
    {
        std::string controllerType;
        if (serviceDescriptor.GetSupplementalDataItem(Core::Discovery::controllerType, controllerType)
              && controllerType == Core::Discovery::controllerTypeRpcServer)
        {
            auto getVal = [serviceDescriptor](std::string key) {
                std::string tmp;
                if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                {
                    throw std::runtime_error{"Unknown key in supplementalData"};
                }
                return tmp;
            };

            auto rpcServerFunctionName = getVal(Core::Discovery::supplKeyRpcServerFunctionName);
            auto rpcServerMediaType = getVal(Core::Discovery::supplKeyRpcServerMediaType);
            std::string labelsStr = getVal(Core::Discovery::supplKeyRpcServerLabels);
            auto rpcServerLabels =
                SilKit::Config::Deserialize<std::map<std::string, std::string>>(labelsStr);

            if ((functionName == "" || functionName == rpcServerFunctionName)
                && MatchMediaType(mediaType, rpcServerMediaType) && MatchLabels(labels, rpcServerLabels))
            {
                discoveryResults.push_back({rpcServerFunctionName, rpcServerMediaType, rpcServerLabels});
            }
        }
    }

    return discoveryResults;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
