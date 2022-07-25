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

#include "RpcClient.hpp"
#include "IServiceDiscovery.hpp"
#include "IParticipantInternal.hpp"
#include "RpcDatatypeUtils.hpp"
#include "UuidRandom.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

RpcClient::RpcClient(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                     const std::string& functionName, const std::string& mediaType,
                     const std::map<std::string, std::string>& labels, const std::string& clientUUID,
                     RpcCallResultHandler handler)
    : _functionName{functionName}
    , _mediaType{mediaType}
    , _labels{labels}
    , _clientUUID{clientUUID}
    , _handler{std::move(handler)}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void RpcClient::RegisterServiceDiscovery()
{
    // The RpcClient discovers RpcServersInternal and is ready to detach calls afterwards
    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
            auto getVal = [serviceDescriptor](std::string key) {
                std::string tmp;
                if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                {
                    throw std::runtime_error{"Unknown key in supplementalData"};
                }
                return tmp;
            };

            auto clientUUID = getVal(Core::Discovery::supplKeyRpcServerInternalClientUUID);

            if (clientUUID == _clientUUID)
            {
                if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                {
                    _numCounterparts++;
                }
                else if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved)
                {
                    _numCounterparts--;
                }
            }
        },
        Core::Discovery::controllerTypeRpcServerInternal, _clientUUID);
}

IRpcCallHandle* RpcClient::Call(Util::Span<const uint8_t> data)
{
    if (_numCounterparts == 0)
    {
        if (_handler)
            _handler(this, RpcCallResultEvent{_timeProvider->Now(), nullptr, RpcCallStatus::ServerNotReachable, {}});
        return nullptr;
    }
    else
    {
        Util::Uuid::UUID uuid = Util::Uuid::generate();
        auto callUUID = CallUUID{uuid.ab, uuid.cd};
        auto callHandle = std::make_unique<CallHandleImpl>(callUUID);
        auto* callHandlePtr = callHandle.get();
        _detachedCallHandles[to_string(callUUID)] = std::make_pair(_numCounterparts, std::move(callHandle));
        FunctionCall msg{_timeProvider->Now(), std::move(callUUID), Util::ToStdVector(data)};
        _participant->SendMsg(this, std::move(msg));
        return callHandlePtr;
    }
}

void RpcClient::SetCallResultHandler(RpcCallResultHandler handler)
{
    _handler = std::move(handler);
}

void RpcClient::ReceiveMsg(const Core::IServiceEndpoint* /*from*/, const FunctionCallResponse& msg)
{
    ReceiveMessage(msg);
}

void RpcClient::ReceiveMessage(const FunctionCallResponse& msg)
{
    auto it = _detachedCallHandles.find(to_string(msg.callUUID));
    if (it != _detachedCallHandles.end())
    {
        if (_handler)
        {
            _handler(this,
                     RpcCallResultEvent{msg.timestamp, (*it).second.second.get(), RpcCallStatus::Success, msg.data});
        }
        // NB: Possibly multiple responses are received (e.g. 1 client, 2 servers).
        // Decrease the count of responses and erase the call handle if all arrived.
        auto* numReceived = &(*it).second.first;
        (*numReceived)--;
        if (*numReceived <= 0)
            _detachedCallHandles.erase(it);
    }
    else
    {
        std::string errorMsg{"RpcClient: Received unknown function call response."};
        _logger->Error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
}

void RpcClient::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
