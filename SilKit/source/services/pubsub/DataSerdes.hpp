// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "WireDataMessages.hpp"

#include "silkit/services/pubsub/DataMessageDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireDataMessageEvent& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireDataMessageEvent& out);

} // namespace PubSub    
} // namespace Services
} // namespace SilKit
