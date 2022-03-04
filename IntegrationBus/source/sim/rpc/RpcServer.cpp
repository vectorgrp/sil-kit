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

RpcServer::RpcServer(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider,
                     const std::string& functionName, const sim::rpc::RpcExchangeFormat& exchangeFormat,
                     const std::map<std::string, std::string>& labels, CallProcessor handler)
    : _functionName{functionName}
    , _exchangeFormat{exchangeFormat}
    , _labels{labels}
    , _handler{std::move(handler)}
    , _logger{comAdapter->GetLogger()}
    , _timeProvider{timeProvider}
    , _comAdapter{comAdapter}
{

}

void RpcServer::RegisterServiceDiscovery()
{
    // RpcServer discovers RpcClient and adds RpcServerInternal on a matching connection
    _comAdapter->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
        [this](ib::mw::service::ServiceDiscoveryEvent::Type discoveryType,
               const ib::mw::ServiceDescriptor& serviceDescriptor) {
            if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated)
            {
                std::string controllerType;
                if (!(serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, controllerType)
                      && controllerType == mw::service::controllerTypeRpcClient))
                {
                    return;
                }

                auto getVal = [serviceDescriptor](std::string key) {
                    std::string tmp;
                    if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                    {
                        throw std::runtime_error{"Unknown key in supplementalData"};
                    }
                    return tmp;
                };

                auto functionName = getVal(mw::service::supplKeyRpcClientFunctionName);
                RpcExchangeFormat clientExchangeFormat{getVal(mw::service::supplKeyRpcClientDxf)};
                auto clientUUID = getVal(mw::service::supplKeyRpcClientUUID);
                std::string labelsStr = getVal(mw::service::supplKeyRpcClientLabels);
                std::map<std::string, std::string> clientLabels =
                    ib::cfg::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (functionName == _functionName && Match(clientExchangeFormat, _exchangeFormat)
                    && MatchLabels(clientLabels, _labels))
                {
                    AddInternalRpcServer(clientUUID, clientExchangeFormat, clientLabels);
                }

            }
        });
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

void RpcServer::AddInternalRpcServer(const std::string& clientUUID, RpcExchangeFormat joinedExchangeFormat,
                                     const std::map<std::string, std::string>& clientLabels)
{
    auto internalRpcServer = dynamic_cast<RpcServerInternal*>(_comAdapter->CreateRpcServerInternal(
         _functionName, clientUUID, joinedExchangeFormat, clientLabels, _handler, this));
    _internalRpcServers.push_back(internalRpcServer);
}

void RpcServer::SetRpcHandler(CallProcessor handler)
{
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
