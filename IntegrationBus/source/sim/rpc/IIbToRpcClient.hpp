// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/rpc/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace rpc {

//! \brief IIbToRpcClient interface used by the ComAdapter
class IIbToRpcClient
    : public mw::IIbEndpoint<ServerAcknowledge, FunctionCallResponse>
    , public mw::IIbSender<ClientAnnouncement, FunctionCall>
{
public:
    virtual ~IIbToRpcClient() noexcept = default;
};

} // namespace rpc
} // namespace sim
} // namespace ib

