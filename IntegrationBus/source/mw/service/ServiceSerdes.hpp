
// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ServiceDescriptor.hpp"
#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {
namespace service {

void Serialize(ib::mw::MessageBuffer& buffer, const ParticipantDiscoveryEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const ServiceDiscoveryEvent& msg);

void Deserialize(MessageBuffer& buffer, ParticipantDiscoveryEvent& out);
void Deserialize(MessageBuffer& buffer, ServiceDiscoveryEvent& out);

} // namespace service    
} // namespace mw
} // namespace ib
