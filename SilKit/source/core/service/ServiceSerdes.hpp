// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "ServiceDescriptor.hpp"
#include "ServiceDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

void Serialize(SilKit::Core::MessageBuffer& buffer, const ParticipantDiscoveryEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const ServiceDiscoveryEvent& msg);

void Deserialize(MessageBuffer& buffer, ParticipantDiscoveryEvent& out);
void Deserialize(MessageBuffer& buffer, ServiceDiscoveryEvent& out);

} // namespace Discovery    
} // namespace Core
} // namespace SilKit
