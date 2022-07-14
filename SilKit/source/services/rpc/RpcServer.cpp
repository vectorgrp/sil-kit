// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
                     const std::string& functionName, const std::string& mediaType,
                     const std::map<std::string, std::string>& labels, RpcCallHandler handler)
    : _functionName{functionName}
    , _mediaType{mediaType}
    , _labels{labels}
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
                std::map<std::string, std::string> clientLabels =
                    SilKit::Config::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (functionName == _functionName && MatchMediaType(clientMediaType, _mediaType)
                    && MatchLabels(clientLabels, _labels))
                {
                    AddInternalRpcServer(clientUUID, clientMediaType, clientLabels);
                }
            }
        }, Core::Discovery::controllerTypeRpcClient, _functionName);
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
                                     const std::map<std::string, std::string>& clientLabels)
{
    auto internalRpcServer = dynamic_cast<RpcServerInternal*>(_participant->CreateRpcServerInternal(
        _functionName, clientUUID, joinedMediaType, clientLabels, _handler, this));
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
