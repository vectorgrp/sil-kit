// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/MessageBuffer.hpp"
#include "wire/pubsub/WireDataMessages.hpp"

#include "silkit/services/pubsub/PubSubDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireDataMessageEvent& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireDataMessageEvent& out);

} // namespace PubSub
} // namespace Services
} // namespace SilKit
