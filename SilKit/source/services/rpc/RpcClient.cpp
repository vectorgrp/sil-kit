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
#include "Uuid.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

namespace {

auto ToRpcCallStatus(const FunctionCallResponse::Status status) -> RpcCallStatus
{
    switch (status)
    {
    case FunctionCallResponse::Status::Success: return RpcCallStatus::Success;
    case FunctionCallResponse::Status::InternalError: return RpcCallStatus::InternalServerError;
    }

    return RpcCallStatus::UndefinedError;
}

} // namespace

RpcClient::RpcClient(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                     const SilKit::Services::Rpc::RpcSpec& dataSpec, const std::string& clientUUID,
                     RpcCallResultHandler handler)
    : _dataSpec{dataSpec}
    , _clientUUID{clientUUID}
    , _handler{std::move(handler)}
    , _logger{participant->GetLogger()}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

RpcClient::~RpcClient()
{
    if (_isTimeoutHandlerSet)
    {
        _timeProvider->RemoveNextSimStepHandler(_timeoutHandlerId);
    }
}

void RpcClient::RegisterServiceDiscovery()
{
    auto matchHandler = [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        auto getVal = [serviceDescriptor](const std::string& key) {
            std::string tmp;
            if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
            {
                throw SilKit::StateError{"Unknown key in supplementalData"};
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
    };

    // The RpcClient discovers RpcServersInternal and is ready to detach calls afterwards
    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        matchHandler, Core::Discovery::controllerTypeRpcServerInternal, _clientUUID, _dataSpec.Labels());
}

void RpcClient::Call(Util::Span<const uint8_t> data, void* userContext)
{
    TriggerCall(std::move(data), false, {}, userContext);
}

void RpcClient::CallWithTimeout(Util::Span<const uint8_t> data, std::chrono::nanoseconds timeout, void* userContext)
{
    TriggerCall(std::move(data), true, timeout, userContext);
}


void RpcClient::TimeHandler(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    std::vector<TimeoutEntry> timeoutedEntries;

    {
        std::unique_lock<decltype(_timeoutQueueMx)> lockTimeout{_timeoutQueueMx};

        for (auto it = _timeoutEntries.begin(); it != _timeoutEntries.end();)
        {
            it->timeLeft -= duration;
            if (it->timeLeft <= static_cast<std::chrono::nanoseconds>(0))
            {
                timeoutedEntries.insert(timeoutedEntries.end(), std::make_move_iterator(it),
                                        std::make_move_iterator(it + 1));
                it = _timeoutEntries.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    for (auto&& entry : timeoutedEntries)
    {
        auto uuid = entry.callUuid;

        std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};
        auto it = _activeCalls.find(uuid);
        auto userContext = it->second.GetUserContext();

        if (it != _activeCalls.end())
        {
            _activeCalls.erase(it);
            lock.unlock();

            _handler(this, RpcCallResultEvent{now, userContext, RpcCallStatus::Timeout, {}});
        }
    }
    timeoutedEntries.clear();
}


void RpcClient::TriggerCall(Util::Span<const uint8_t> data, bool hasTimeout, std::chrono::nanoseconds timeout, void* userContext)
{
    if (_numCounterparts == 0)
    {
        if (_handler)
        {
            _handler(this,
                     RpcCallResultEvent{_timeProvider->Now(), userContext, RpcCallStatus::ServerNotReachable, {}});
        }
    }
    else
    {
        const auto callUuid = Util::Uuid::GenerateRandom();

        FunctionCall msg{_timeProvider->Now(), callUuid, Util::ToStdVector(data)};

        {
            {
                std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};
                _activeCalls.emplace(callUuid, RpcCallInfo{static_cast<int32_t>(_numCounterparts), userContext});
            }

            if (hasTimeout)
            {
                {
                    std::unique_lock<decltype(_timeoutQueueMx)> lockTimeout{_timeoutQueueMx};
                    _timeoutEntries.push_back({timeout, callUuid});
                }

                if (!_isTimeoutHandlerSet)
                {
                    _timeoutHandlerId = _timeProvider->AddNextSimStepHandler(
                        [this](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                            this->TimeHandler(now, duration);
                        });
                    _isTimeoutHandlerSet = true;
                }
            }
        }

        _participant->SendMsg(this, std::move(msg));
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
    std::map<SilKit::Util::Uuid, SilKit::Services::Rpc::RpcClient::RpcCallInfo>::iterator it;
    {
        std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};

        it = _activeCalls.find(msg.callUuid);

        if (it == _activeCalls.end())
        {
            std::string warningMsg{"RpcClient: Received function call response with an unknown/deleted uuid. Might be a call reply that ran into a timeout."};
            _logger->Warn(warningMsg);
            return;
        }
    };

    if (_handler)
    {
        _handler(this,
                 RpcCallResultEvent{msg.timestamp, it->second.GetUserContext(), ToRpcCallStatus(msg.status), msg.data});
    }

    // NB: If the call was made to multiple servers, multiple returns will be received. Only forget about the call
    //     after all returns have been received.
    if (it->second.DecrementRemainingReturnCount() <= 0)
    {
        std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};
        _activeCalls.erase(it);
    }
}

void RpcClient::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
