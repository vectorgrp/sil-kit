// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <vector>
#include <map>

#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/sim/rpc/IRpcServer.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbToRpcServerInternal.hpp"
#include "RpcCallHandle.hpp"

namespace ib {
namespace sim {
namespace rpc {

class RpcServerInternal : public IIbToRpcServerInternal,
                  public mw::sync::ITimeConsumer,
                  public mw::IIbServiceEndpoint
{
  public:
    RpcServerInternal(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider,
                      const std::string& functionName, const sim::rpc::RpcExchangeFormat& exchangeFormat,
                      const std::map<std::string, std::string>& labels, const std::string& clientUUID,
                      ib::sim::rpc::CallProcessor handler, IRpcServer* parent);

    void SetRpcHandler(CallProcessor handler);

    void SubmitResult(IRpcCallHandle* callHandlePtr, const std::vector<uint8_t>& resultData);

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const FunctionCall& msg) override;
    void ReceiveMessage(const FunctionCall& msg);

    // ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

  private:

    std::string _functionName;
    sim::rpc::RpcExchangeFormat _exchangeFormat;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;
    CallProcessor _handler;
    IRpcServer* _parent;

    cfg::RpcPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};

    std::map<std::string, std::unique_ptr<CallHandleImpl>> _receivedCallHandles;

};

// ================================================================================
//  Inline Implementations
// ================================================================================

void RpcServerInternal::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RpcServerInternal::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace rpc
} // namespace sim
} // namespace ib
