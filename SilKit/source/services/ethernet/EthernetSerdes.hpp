// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "WireEthernetMessages.hpp"

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireEthernetFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const EthernetFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const EthernetStatus& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const EthernetSetMode& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, WireEthernetFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, EthernetFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, EthernetStatus& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, EthernetSetMode& out);

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
