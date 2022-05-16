// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "RpcServer.hpp"
#include "RpcDatatypeUtils.hpp"
#include "UuidRandom.hpp"
#include "YamlParser.hpp"

namespace ib {
namespace sim {
namespace rpc {

RpcServer::RpcServer(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
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
        [this](ib::mw::service::ServiceDiscoveryEvent::Type discoveryType,
               const ib::mw::ServiceDescriptor& serviceDescriptor) {
            if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated)
            {

                auto getVal = [serviceDescriptor](std::string key) {
                    std::string tmp;
                    if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                    {
                        throw std::runtime_error{"Unknown key in supplementalData"};
                    }
                    return tmp;
                };

                auto functionName = getVal(mw::service::supplKeyRpcClientFunctionName);
                auto clientMediaType = getVal(mw::service::supplKeyRpcClientMediaType);
                auto clientUUID = getVal(mw::service::supplKeyRpcClientUUID);
                std::string labelsStr = getVal(mw::service::supplKeyRpcClientLabels);
                std::map<std::string, std::string> clientLabels =
                    ib::cfg::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (functionName == _functionName && MatchMediaType(clientMediaType, _mediaType)
                    && MatchLabels(clientLabels, _labels))
                {
                    AddInternalRpcServer(clientUUID, clientMediaType, clientLabels);
                }
            }
        }, mw::service::controllerTypeRpcClient, _functionName);
}

void RpcServer::SubmitResult(IRpcCallHandle* callHandle, std::vector<uint8_t> resultData)
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

void RpcServer::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace rpc
} // namespace sim
} // namespace ib
