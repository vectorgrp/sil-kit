// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/MessageBuffer.hpp"
#include "core/internal/ServiceDescriptor.hpp"
#include "core/service/ServiceDatatypes.hpp"

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
