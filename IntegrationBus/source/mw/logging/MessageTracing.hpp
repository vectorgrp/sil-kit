// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace mw {

template<class IbMessageT>
void TraceRx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Recv from {}: {}", addr, msg);
}

template<class IbMessageT>
void TraceTx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Send from {}: {}", addr, msg);
}

// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}
inline void TraceTx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}

} // namespace mw
} // namespace ib

