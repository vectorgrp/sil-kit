// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>
#include <set>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/rpc/IRpcClient.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"
#include "ib/sim/rpc/string_utils.hpp"
#include "ITimeConsumer.hpp"

#include "IIbToRpcClient.hpp"
#include "IParticipantInternal.hpp"
#include "RpcCallHandle.hpp"

namespace ib {
namespace sim {
namespace rpc {

class RpcClient
    : public IRpcClient
    , public IIbToRpcClient
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint

{
public:
    RpcClient(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
              const std::string& functionName, const std::string& mediaType,
              const std::map<std::string, std::string>& labels, const std::string& clientUUID,
              RpcCallResultHandler handler);

    void RegisterServiceDiscovery();

    auto Call(std::vector<uint8_t> data) -> IRpcCallHandle* override;
    auto Call(const uint8_t* data, std::size_t size) -> IRpcCallHandle* override;

    void SetCallResultHandler(RpcCallResultHandler handler) override;

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const FunctionCallResponse& msg) override;
    void ReceiveMessage(const FunctionCallResponse& msg);

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

private:
    std::string _functionName;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;

    RpcCallResultHandler _handler;

    mw::ServiceDescriptor _serviceDescriptor{};
    uint32_t _numCounterparts{0};
    std::map<std::string, std::pair<uint32_t, std::unique_ptr<CallHandleImpl>>> _detachedCallHandles;
    mw::logging::ILogger* _logger;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcClient::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcClient::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace rpc
} // namespace sim
} // namespace ib
