// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

#include "ib/sim/rpc/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace rpc {

//! \brief IIbToRpcClient interface used by the ComAdapter
class IIbToRpcClient
    : public mw::IIbReceiver<FunctionCallResponse>
    , public mw::IIbSender<FunctionCall>
{
public:
    virtual ~IIbToRpcClient() noexcept = default;
};

} // namespace rpc
} // namespace sim
} // namespace ib

