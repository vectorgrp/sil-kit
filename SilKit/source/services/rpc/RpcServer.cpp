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

#include "silkit/services/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "RpcServer.hpp"
#include "RpcDatatypeUtils.hpp"
#include "UuidRandom.hpp"
#include "YamlParser.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcServer::RpcServer(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                     const SilKit::Services::Rpc::RpcServerSpec& dataSpec, RpcCallHandler handler)
    : _dataSpec{dataSpec}
    , _handler{std::move(handler)}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void RpcServer::RegisterServiceDiscovery()
{
    // RpcServer discovers RpcClient and adds RpcServerInternal on a matching connection
    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
            if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
            {

                auto getVal = [serviceDescriptor](std::string key) {
                    std::string tmp;
                    if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                    {
                        throw std::runtime_error{"Unknown key in supplementalData"};
                    }
                    return tmp;
                };

                auto functionName = getVal(Core::Discovery::supplKeyRpcClientFunctionName);
                auto clientMediaType = getVal(Core::Discovery::supplKeyRpcClientMediaType);
                auto clientUUID = getVal(Core::Discovery::supplKeyRpcClientUUID);
                std::string labelsStr = getVal(Core::Discovery::supplKeyRpcClientLabels);
                auto clientLabels =
                    SilKit::Config::Deserialize<std::vector<SilKit::Services::Label>>(labelsStr);

                if (functionName == _dataSpec.Topic() && MatchMediaType(clientMediaType, _dataSpec.MediaType())
                    && MatchLabels(_dataSpec.Labels(), clientLabels))
                {
                    AddInternalRpcServer(clientUUID, clientMediaType, clientLabels);
                }
            }
        }, Core::Discovery::controllerTypeRpcClient, _dataSpec.Topic());
}

void RpcServer::SubmitResult(IRpcCallHandle* callHandle, Util::Span<const uint8_t> resultData)
{
    if (callHandle != nullptr)
    {
        for (auto* internalRpcServer : _internalRpcServers)
        {
            internalRpcServer->SubmitResult(callHandle, resultData);
        }
    }
    else
    {
        std::string errorMsg{"RpcServer::SubmitResult() must not be called with an invalid call handle!"};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
}

void RpcServer::AddInternalRpcServer(const std::string& clientUUID, std::string joinedMediaType,
                                     const std::vector<SilKit::Services::Label>& clientLabels)
{
    auto internalRpcServer = dynamic_cast<RpcServerInternal*>(_participant->CreateRpcServerInternal(
        _dataSpec.Topic(), clientUUID, joinedMediaType, clientLabels, _handler, this));
    _internalRpcServers.push_back(internalRpcServer);
}

void RpcServer::SetCallHandler(RpcCallHandler handler)
{
    _handler = handler;
    for (auto* internalRpcServer : _internalRpcServers)
    {
        internalRpcServer->SetRpcHandler(handler);
    }
}

void RpcServer::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
