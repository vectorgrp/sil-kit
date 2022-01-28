// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>

#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

bool operator==(const CallUUID& lhs, const CallUUID& rhs);
bool operator!=(const CallUUID& lhs, const CallUUID& rhs);

bool operator==(const FunctionCall& lhs, const FunctionCall& rhs);
bool operator==(const FunctionCallResponse& lhs, const FunctionCallResponse& rhs);

bool Match(const RpcExchangeFormat& lhs, const RpcExchangeFormat& rhs);

} // namespace rpc
} // namespace sim
} // namespace ib
