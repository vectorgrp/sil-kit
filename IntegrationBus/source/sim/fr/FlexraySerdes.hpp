// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/fr/FlexrayDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayFrameEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayFrameTransmitEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexraySymbolEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayCycleStartEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayHostCommand& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayControllerConfig& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayTxBufferUpdate& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const FlexrayPocStatusEvent& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayFrameEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayFrameTransmitEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexraySymbolEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexraySymbolTransmitEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayCycleStartEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayHostCommand& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayControllerConfig& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayTxBufferUpdate& out);
void Deserialize(ib::mw::MessageBuffer& buffer, FlexrayPocStatusEvent& out);

} // namespace fr    
} // namespace sim
} // namespace ib
