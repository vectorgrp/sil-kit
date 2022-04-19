// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcDiscoverer.hpp"
#include "RpcDatatypeUtils.hpp"
#include "YamlParser.hpp"

namespace ib {
namespace sim {
namespace rpc {

RpcDiscoverer::RpcDiscoverer(mw::service::IServiceDiscovery* serviceDiscovery) : _serviceDiscovery { serviceDiscovery }
{
}

std::vector<RpcDiscoveryResult> RpcDiscoverer::GetMatchingRpcServers(
    const std::string& rpcChannel, const sim::rpc::RpcExchangeFormat& exchangeFormat,
    const std::map<std::string, std::string>& labels) const
{
    std::vector<RpcDiscoveryResult> discoveryResults;
    auto serviceDescriptors = _serviceDiscovery->GetServices();

    for (const auto& serviceDescriptor : serviceDescriptors)
    {
        std::string controllerType;
        if (serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, controllerType)
              && controllerType == mw::service::controllerTypeRpcServer)
        {
            auto getVal = [serviceDescriptor](std::string key) {
                std::string tmp;
                if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                {
                    throw std::runtime_error{"Unknown key in supplementalData"};
                }
                return tmp;
            };

            auto rpcServerFunctionName = getVal(mw::service::supplKeyRpcServerFunctionName);
            RpcExchangeFormat rpcServerExchangeFormat{getVal(mw::service::supplKeyRpcServerDxf)};
            std::string labelsStr = getVal(mw::service::supplKeyRpcServerLabels);
            auto rpcServerLabels =
                ib::cfg::Deserialize<std::map<std::string, std::string>>(labelsStr);

            if ((rpcChannel == "" || rpcChannel == rpcServerFunctionName)
                && Match(exchangeFormat, rpcServerExchangeFormat) && MatchLabels(labels, rpcServerLabels))
            {
                discoveryResults.push_back({rpcServerFunctionName, rpcServerExchangeFormat, rpcServerLabels});
            }
        }
    }

    return discoveryResults;
}

} // namespace rpc
} // namespace sim
} // namespace ib
