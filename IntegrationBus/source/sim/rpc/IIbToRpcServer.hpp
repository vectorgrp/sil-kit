// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/rpc/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace rpc {

//! \brief IIbToRpcServer interface used by the ComAdapter
class IIbToRpcServer
    : public mw::IIbEndpoint<ClientAnnouncement>
    , public mw::IIbSender<ServerAcknowledge>
{
public:
    virtual ~IIbToRpcServer() noexcept = default;
};

} // namespace rpc
} // namespace sim
} // namespace ib

