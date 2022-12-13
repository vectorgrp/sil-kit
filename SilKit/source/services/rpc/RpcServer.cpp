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
#include "Uuid.hpp"
#include "YamlParser.hpp"
#include "Assert.hpp"
#include "LabelMatching.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcServer::RpcServer(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                     const SilKit::Services::Rpc::RpcSpec& dataSpec, RpcCallHandler handler)
    : _dataSpec{dataSpec}
    , _handler{std::move(handler)}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void RpcServer::RegisterServiceDiscovery()
{
    auto matchHandler = [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
        {
            auto getVal = [serviceDescriptor](const std::string& key) {
                std::string tmp;
                if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                {
                    throw SilKit::StateError{"Unknown key in supplementalData"};
                }
                return tmp;
            };

            auto functionName = getVal(Core::Discovery::supplKeyRpcClientFunctionName);
            auto clientMediaType = getVal(Core::Discovery::supplKeyRpcClientMediaType);
            auto clientUUID = getVal(Core::Discovery::supplKeyRpcClientUUID);
            std::string labelsStr = getVal(Core::Discovery::supplKeyRpcClientLabels);
            auto clientLabels = SilKit::Config::Deserialize<std::vector<SilKit::Services::MatchingLabel>>(labelsStr);

            if (functionName == _dataSpec.FunctionName() && MatchMediaType(clientMediaType, _dataSpec.MediaType())
                && Util::MatchLabels(_dataSpec.Labels(), clientLabels))
            {
                AddInternalRpcServer(clientUUID, clientMediaType, clientLabels);
            }
        }
    };

    // How this controller is discovered by RpcClient
    const std::string discoveryLookupKey = Core::Discovery::controllerTypeRpcClient + "/"
                          + Core::Discovery::supplKeyRpcClientFunctionName + "/" + _dataSpec.FunctionName();

    // RpcServer discovers RpcClient and adds RpcServerInternal on a matching connection
    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        matchHandler, Core::Discovery::controllerTypeRpcClient, _dataSpec.FunctionName(), _dataSpec.Labels());
}

void RpcServer::SubmitResult(IRpcCallHandle* callHandle, Util::Span<const uint8_t> resultData)
{
    if (callHandle == nullptr)
    {
        std::string errorMsg = "RpcServer::SubmitResult() must not be called with an invalid call handle!";
        _logger->Error(errorMsg);
        throw SilKit::StateError{std::move(errorMsg)};
    }

    // counts the number of RpcServerInternal's living within this RpcServer that returned the FunctionCall
    uint32_t submitResultCounter = 0;

    {
        std::unique_lock<decltype(_internalRpcServersMx)> lock{_internalRpcServersMx};
        for (auto* internalRpcServer : _internalRpcServers)
        {
            submitResultCounter += (internalRpcServer->SubmitResult(callHandle, resultData) ? 1 : 0);
        }
    }

    if (submitResultCounter != 1)
    {
        // NB: Multiple returns are possible, but only from _different_ RpcServers
        std::string errorMsg = "RpcServer::SubmitResult() returned to multiple clients";
        _logger->Error(errorMsg);
        throw SilKit::StateError{std::move(errorMsg)};
    }
}

void RpcServer::AddInternalRpcServer(const std::string& clientUUID, std::string joinedMediaType,
                                     const std::vector<SilKit::Services::MatchingLabel>& clientLabels)
{
    auto internalRpcServer = dynamic_cast<RpcServerInternal*>(_participant->CreateRpcServerInternal(
        _dataSpec.FunctionName(), clientUUID, joinedMediaType, clientLabels, _handler, this));

    std::unique_lock<decltype(_internalRpcServersMx)> lock{_internalRpcServersMx};
    _internalRpcServers.push_back(internalRpcServer);
}

void RpcServer::SetCallHandler(RpcCallHandler handler)
{
    _handler = handler;

    std::unique_lock<decltype(_internalRpcServersMx)> lock{_internalRpcServersMx};
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
