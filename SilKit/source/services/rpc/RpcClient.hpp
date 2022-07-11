// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>
#include <set>

#include "silkit/services/rpc/IRpcClient.hpp"
#include "silkit/services/rpc/IRpcCallHandle.hpp"
#include "silkit/services/rpc/string_utils.hpp"

#include "ITimeConsumer.hpp"
#include "IMsgForRpcClient.hpp"
#include "IParticipantInternal.hpp"
#include "RpcCallHandle.hpp"

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
              const std::string& functionName, const std::string& mediaType,
              const std::map<std::string, std::string>& labels, const std::string& clientUUID,
              RpcCallResultHandler handler);

    void RegisterServiceDiscovery();

    auto Call(std::vector<uint8_t> data) -> IRpcCallHandle* override;
    auto Call(const uint8_t* data, std::size_t size) -> IRpcCallHandle* override;

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
    std::string _functionName;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;

    RpcCallResultHandler _handler;

    Core::ServiceDescriptor _serviceDescriptor{};
    uint32_t _numCounterparts{0};
    std::map<std::string, std::pair<uint32_t, std::unique_ptr<CallHandleImpl>>> _detachedCallHandles;
    Services::Logging::ILogger* _logger;
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
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
