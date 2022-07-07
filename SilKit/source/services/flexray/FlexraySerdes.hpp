// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayCycleStartEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayHostCommand& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayControllerConfig& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferUpdate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const FlexrayPocStatusEvent& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexraySymbolTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayCycleStartEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayHostCommand& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayControllerConfig& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferUpdate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, FlexrayPocStatusEvent& out);

} // namespace Flexray    
} // namespace Services
} // namespace SilKit
