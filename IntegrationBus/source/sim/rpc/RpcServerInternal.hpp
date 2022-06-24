// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <vector>
#include <map>

#include "ib/mw/fwd_decl.hpp"
#include "ITimeConsumer.hpp"
#include "ib/sim/rpc/IRpcServer.hpp"
#include "ib/sim/rpc/IRpcCallHandle.hpp"

#include "IParticipantInternal.hpp"
#include "IIbToRpcServerInternal.hpp"
#include "RpcCallHandle.hpp"

namespace ib {
namespace sim {
namespace rpc {

class RpcServerInternal
    : public IIbToRpcServerInternal
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    RpcServerInternal(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
                      const std::string& functionName, const std::string& mediaType,
                      const std::map<std::string, std::string>& labels, const std::string& clientUUID,
                      ib::sim::rpc::RpcCallHandler handler, IRpcServer* parent);

    void SetRpcHandler(RpcCallHandler handler);

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
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    std::string _clientUUID;
    RpcCallHandler _handler;
    IRpcServer* _parent;

    mw::ServiceDescriptor _serviceDescriptor{};
    std::map<std::string, std::unique_ptr<CallHandleImpl>> _receivedCallHandles;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::IParticipantInternal* _participant{nullptr};
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
