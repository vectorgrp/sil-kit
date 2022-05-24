// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/can/CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {

void Serialize(ib::mw::MessageBuffer& buffer,const sim::can::CanFrameEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer,const sim::can::CanFrameTransmitEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer,const sim::can::CanControllerStatus& msg);
void Serialize(ib::mw::MessageBuffer& buffer,const sim::can::CanConfigureBaudrate& msg);
void Serialize(ib::mw::MessageBuffer& buffer,const sim::can::CanSetControllerMode& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, sim::can::CanFrameEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, sim::can::CanFrameTransmitEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, sim::can::CanControllerStatus& out);
void Deserialize(ib::mw::MessageBuffer& buffer, sim::can::CanConfigureBaudrate& out);
void Deserialize(ib::mw::MessageBuffer& buffer, sim::can::CanSetControllerMode& out);

} // namespace can    
} // namespace sim
} // namespace ib
