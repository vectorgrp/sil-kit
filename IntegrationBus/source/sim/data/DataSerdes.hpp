// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {
void Serialize(ib::mw::MessageBuffer& buffer, const DataMessageEvent& msg);
void Deserialize(ib::mw::MessageBuffer& buffer, DataMessageEvent& out);
} // namespace data    
} // namespace sim
} // namespace ib
