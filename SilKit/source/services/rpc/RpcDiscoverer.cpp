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
