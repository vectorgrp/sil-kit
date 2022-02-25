// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

#include "ib/sim/rpc/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace rpc {

//! \brief IIbToRpcServer interface used by the ComAdapter
class IIbToRpcServerInternal
    : public mw::IIbReceiver<FunctionCall>
    , public mw::IIbSender<FunctionCallResponse>
{
public:
    virtual ~IIbToRpcServerInternal() noexcept = default;
};

} // namespace rpc
} // namespace sim
} // namespace ib

