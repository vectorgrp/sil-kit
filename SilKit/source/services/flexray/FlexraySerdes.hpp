// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MessageBuffer.hpp"
#include "WireFlexrayMessages.hpp"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayCycleStartEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayHostCommand& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayControllerConfig& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WireFlexrayTxBufferUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayPocStatusEvent& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayCycleStartEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayHostCommand& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayControllerConfig& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WireFlexrayTxBufferUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayPocStatusEvent& out);

} // namespace Flexray
} // namespace Services
} // namespace SilKit
