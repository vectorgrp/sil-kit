// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "RequestReplyDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

void Serialize(SilKit::Core::MessageBuffer& buffer, const RequestReplyCall& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const RequestReplyCallReturn& msg);

void Deserialize(MessageBuffer& buffer, RequestReplyCall& out);
void Deserialize(MessageBuffer& buffer, RequestReplyCallReturn& out);

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
