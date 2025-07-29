// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <future>
#include <queue>
#include <set>

#include "silkit/services/rpc/IRpcClient.hpp"
#include "silkit/services/rpc/IRpcCallHandle.hpp"
#include "silkit/services/rpc/string_utils.hpp"

#include "ITimeConsumer.hpp"
#include "ITimeProvider.hpp"
#include "IMsgForRpcClient.hpp"
#include "IParticipantInternal.hpp"
#include "RpcCallHandle.hpp"
#include "Uuid.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class RpcClient
    : public IRpcClient
    , public IMsgForRpcClient
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint

{
public:
    RpcClient(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
              const SilKit::Services::Rpc::RpcSpec& dataSpec, const std::string& clientUUID,
              RpcCallResultHandler handler);
    ~RpcClient();

    void RegisterServiceDiscovery();

    void Call(Util::Span<const uint8_t> data, void* userContext = nullptr) override;
    void CallWithTimeout(Util::Span<const uint8_t> data, std::chrono::nanoseconds timeout,
                         void* userContext = nullptr) override;

    void SetCallResultHandler(RpcCallResultHandler handler) override;

    //! \brief Accepts messages originating from SIL Kit communications.
    void ReceiveMsg(const Core::IServiceEndpoint* from, const FunctionCallResponse& msg) override;
    void ReceiveMessage(const FunctionCallResponse& msg);

    //SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

private:
    void TriggerCall(Util::Span<const uint8_t> data, bool hasTimeout, std::chrono::nanoseconds timeout,
                     void* userContext);
    void TimeHandler(std::chrono::nanoseconds now, std::chrono::nanoseconds duration);

    class RpcCallInfo
    {
    public:
        RpcCallInfo(int32_t remainingReturnCount, void* userContext)
            : _remainingReturnCount{remainingReturnCount}
            , _userContext{userContext}
        {
        }

        auto DecrementRemainingReturnCount() -> int32_t
        {
            return --_remainingReturnCount;
        }

        auto GetUserContext() const -> void*
        {
            return _userContext;
        }

    private:
        int32_t _remainingReturnCount = 0;
        void* _userContext = nullptr;
    };

    SilKit::Services::Rpc::RpcSpec _dataSpec;
    std::string _clientUUID;

    RpcCallResultHandler _handler;

    Core::ServiceDescriptor _serviceDescriptor{};
    std::atomic<uint32_t> _numCounterparts{0};
    std::map<std::string, std::pair<uint32_t, std::unique_ptr<RpcCallHandle>>> _detachedCallHandles;
    Services::Logging::ILogger* _logger;
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};

    std::mutex _activeCallsMx;
    std::mutex _timeoutQueueMx;
    std::map<Util::Uuid, RpcCallInfo> _activeCalls;

    struct TimeoutEntry
    {
        std::chrono::nanoseconds timeLeft;
        Util::Uuid callUuid;
    };


    std::vector<TimeoutEntry> _timeoutEntries{};
    std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> _timeoutHandler{};
    Services::HandlerId _timeoutHandlerId{};
    std::atomic<bool> _isTimeoutHandlerSet{false};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcClient::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcClient::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
