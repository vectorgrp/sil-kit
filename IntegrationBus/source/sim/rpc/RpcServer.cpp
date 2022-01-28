// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/mw/logging/ILogger.hpp"

#include "RpcServer.hpp"
#include "RpcDatatypeUtils.hpp"
#include "UuidRandom.hpp"

namespace ib {
namespace sim {
namespace rpc {

RpcServer::RpcServer(mw::IComAdapterInternal* comAdapter, cfg::RpcPort config, mw::sync::ITimeProvider* timeProvider, CallProcessor handler)
  : _comAdapter{comAdapter}, _timeProvider{timeProvider}, _handler{std::move(handler)}, _logger{ comAdapter->GetLogger() }
{
    _config = std::move(config);
}

auto RpcServer::Config() const -> const cfg::RpcPort&
{
    return _config;
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

void RpcServer::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const ClientAnnouncement& msg)
{
    ReceiveMessage(msg);
}

void RpcServer::ReceiveMessage(const ClientAnnouncement& msg)
{
    if (msg.functionName == _config.name && 
        Match(msg.exchangeFormat, _config.exchangeFormat))
    {
        _config.clientUUID = msg.clientUUID;
        AddInternalRpcServer();
    }
}

void RpcServer::AddInternalRpcServer()
{
    auto internalRpcServer = dynamic_cast<RpcServerInternal*>(_comAdapter->CreateRpcServerInternal(
         _config.name, _config.clientUUID, _config.exchangeFormat, _handler, this));

    _internalRpcServers.push_back(internalRpcServer);
    SendServerAcknowledge();
}

void RpcServer::SendServerAcknowledge()
{
    ServerAcknowledge msg{ _config.clientUUID };
    _comAdapter->SendIbMessage(this, std::move(msg));
}

void RpcServer::SetRpcHandler(CallProcessor handler)
{
    for (auto* internalRpcServer : _internalRpcServers)
    {
        internalRpcServer->SetRpcHandler(handler);
    }
}

void RpcServer::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}

auto RpcServer::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

void RpcServer::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace rpc
} // namespace sim
} // namespace ib
