// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>
#include <set>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/rpc/IRpcClient.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"
#include "ib/sim/rpc/string_utils.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToRpcClient.hpp"
#include "IComAdapterInternal.hpp"
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
    RpcClient(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider,
              const std::string& functionName, const sim::rpc::RpcExchangeFormat& exchangeFormat,
              const std::map<std::string, std::string>& labels, const std::string& clientUUID,
              CallReturnHandler handler);

    void RegisterServiceDiscovery();

    auto Call(std::vector<uint8_t> data) -> IRpcCallHandle* override;
    auto Call(const uint8_t* data, std::size_t size) -> IRpcCallHandle* override;

    void SetCallReturnHandler(CallReturnHandler handler) override;

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
    sim::rpc::RpcExchangeFormat _exchangeFormat;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;

    CallReturnHandler _handler;
    RpcClient* _callController;

    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    uint32_t _numCounterparts{0};
    std::map<std::string, std::pair<uint32_t, std::unique_ptr<CallHandleImpl>>> _detachedCallHandles;
    mw::logging::ILogger* _logger;

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
