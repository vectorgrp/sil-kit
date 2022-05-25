// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

void Serialize(ib::mw::MessageBuffer& buffer,const FunctionCall& msg);
void Serialize(ib::mw::MessageBuffer& buffer,const FunctionCallResponse& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, FunctionCall& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FunctionCallResponse& out);
} // namespace rpc
} // namespace sim
} // namespace ib
