// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "ib/mw/logging/LoggingDatatypes.hpp"

namespace ib {
namespace mw {
namespace logging {

void Serialize(ib::mw::MessageBuffer& buffer,const LogMsg& msg);
void Deserialize(ib::mw::MessageBuffer& buffer, LogMsg& out);

} // namespace logging
} // namespace mw
} // namespace ib
