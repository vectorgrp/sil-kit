// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {
void Serialize(ib::mw::MessageBuffer& buffer, const LinFrame& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinSendFrameRequest& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinSendFrameHeaderRequest& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinTransmission& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinWakeupPulse& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinControllerConfig& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinControllerStatusUpdate& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinFrameResponse& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const LinFrameResponseUpdate& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, LinFrame& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinSendFrameRequest& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinSendFrameHeaderRequest& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinTransmission& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinWakeupPulse& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinControllerConfig& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinControllerStatusUpdate& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinFrameResponse& out);
void Deserialize(ib::mw::MessageBuffer& buffer, LinFrameResponseUpdate& out);
} // namespace lin    
} // namespace sim
} // namespace ib
