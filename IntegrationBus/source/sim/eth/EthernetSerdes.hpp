// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/eth/EthernetDatatypes.hpp"

namespace ib {
namespace sim {
namespace eth {

void Serialize(ib::mw::MessageBuffer& buffer, const EthernetFrameEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const EthernetFrameTransmitEvent& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const EthernetStatus& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const EthernetSetMode& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, EthernetFrameEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, EthernetFrameTransmitEvent& out);
void Deserialize(ib::mw::MessageBuffer& buffer, EthernetStatus& out);
void Deserialize(ib::mw::MessageBuffer& buffer, EthernetSetMode& out);


} // namespace eth    
} // namespace sim
} // namespace ib
