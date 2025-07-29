// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "WireRpcMessages.hpp"

#include "silkit/services/rpc/RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

void Serialize(SilKit::Core::MessageBuffer& buffer, const FunctionCall& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FunctionCallResponse& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, FunctionCall& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FunctionCallResponse& out);

} // namespace Rpc
} // namespace Services
} // namespace SilKit
