// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"

#include "ib/mw/string_utils.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/eth/string_utils.hpp"
#include "ib/sim/fr/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/sim/io/string_utils.hpp"
#include "ib/sim/generic/string_utils.hpp"

namespace ib {
namespace mw {

template<class IbMessageT>
void TraceRx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Recv from {}: {}", addr, to_string(msg));
}

template<class IbMessageT>
void TraceTx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Send from {}: {}", addr, to_string(msg));
}

// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}
inline void TraceTx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}

} // namespace mw
} // namespace ib

