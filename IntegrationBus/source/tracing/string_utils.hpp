// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/extensions/TraceMessage.hpp"

namespace ib {
namespace tracing {

// string utilities 
std::ostream& operator<<(std::ostream& out, const TraceMessage&);
std::string to_string(const TraceMessage&);

std::ostream& operator<<(std::ostream& out, TraceMessageType);
std::string to_string(TraceMessageType);

} //end namespace tracing
} //end namespace ib
