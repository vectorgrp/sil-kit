// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <algorithm>

#include "services/logging/ILoggerInternal.hpp"
#include "services/logging/LoggerMessage.hpp"

#include "services/rpc/RpcClient.hpp"
#include "core/service/IServiceDiscovery.hpp"
#include "core/internal/IParticipantInternal.hpp"
#include "services/rpc/RpcDatatypeUtils.hpp"
#include "util/Uuid.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

namespace {

auto ToRpcCallStatus(const FunctionCallResponse::Status status) -> RpcCallStatus
{
    switch (status)
    {
    case FunctionCallResponse::Status::Success:
        return RpcCallStatus::Success;
    case FunctionCallResponse::Status::InternalError:
        return RpcCallStatus::InternalServerError;
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
    , _logger{participant->GetLoggerInternal()}
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
        auto clientUUID =
            serviceDescriptor.GetSupplementalDataValue(Core::Discovery::supplKeyRpcServerInternalClientUUID);

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
    TriggerCall(std::move(data), std::nullopt, userContext);
}

void RpcClient::CallWithTimeout(Util::Span<const uint8_t> data, std::chrono::nanoseconds timeout, void* userContext)
{
    TriggerCall(std::move(data), timeout, userContext);
}


void RpcClient::TimeHandler(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    std::vector<void*> timedOutUserContexts;

    {
        std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};

        for (auto it = _timeoutEntries.begin(); it != _timeoutEntries.end();)
        {
            it->timeLeft -= duration;
            if (it->timeLeft <= static_cast<std::chrono::nanoseconds>(0))
            {
                const auto call = _activeCalls.find(it->callUuid);
                if (call != _activeCalls.end())
                {
                    timedOutUserContexts.push_back(call->second.GetUserContext());
                    _activeCalls.erase(call);
                }
                it = _timeoutEntries.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    if (_handler)
    {
        for (auto* userContext : timedOutUserContexts)
        {
            _handler(this, RpcCallResultEvent{now, userContext, RpcCallStatus::Timeout, {}});
        }
    }
}


void RpcClient::TriggerCall(Util::Span<const uint8_t> data, std::optional<std::chrono::nanoseconds> timeout,
                            void* userContext)
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
            std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};
            _activeCalls.emplace(callUuid, RpcCallInfo{static_cast<int32_t>(_numCounterparts), userContext});
            if (timeout)
            {
                _timeoutEntries.push_back({*timeout, callUuid});
            }
        }

        // NB: The time provider invokes the handler while holding its own lock, which then takes _activeCallsMx.
        //     Register outside of _activeCallsMx to keep that lock order.
        if (timeout && !_isTimeoutHandlerSet.exchange(true))
        {
            _timeoutHandlerId = _timeProvider->AddNextSimStepHandler(
                [this](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                this->TimeHandler(now, duration);
            });
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
    void* userContext{nullptr};
    {
        std::unique_lock<decltype(_activeCallsMx)> lock{_activeCallsMx};

        const auto it = _activeCalls.find(msg.callUuid);

        if (it == _activeCalls.end())
        {
            _logger->MakeMessage(Services::Logging::Level::Warn, TopicOf(*this))
                .SetMessage("RpcClient: Received function call response with an unknown/deleted uuid. Might be "
                            "a call reply that ran into a timeout.")
                .Dispatch();
            return;
        }

        userContext = it->second.GetUserContext();

        // NB: If the call was made to multiple servers, multiple returns will be received. Only forget about the call
        //     after all returns have been received.
        if (it->second.DecrementRemainingReturnCount() <= 0)
        {
            _activeCalls.erase(it);
            _timeoutEntries.erase(std::remove_if(_timeoutEntries.begin(), _timeoutEntries.end(),
                                                 [&msg](const auto& entry) { return entry.callUuid == msg.callUuid; }),
                                  _timeoutEntries.end());
        }
    }

    if (_handler)
    {
        _handler(this, RpcCallResultEvent{msg.timestamp, userContext, ToRpcCallStatus(msg.status), msg.data});
    }
}

void RpcClient::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
