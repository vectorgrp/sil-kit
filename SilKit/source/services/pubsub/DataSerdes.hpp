// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "silkit/services/pubsub/DataMessageDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {
void Serialize(SilKit::Core::MessageBuffer& buffer, const DataMessageEvent& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, DataMessageEvent& out);
} // namespace PubSub    
} // namespace Services
} // namespace SilKit
