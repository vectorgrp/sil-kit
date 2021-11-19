// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"
#include "../internal/IServiceId.hpp"

namespace ib {
namespace mw {

template<class IbMessageT>
//[[deprecated]]
void TraceRx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Recv from {}: {}", addr, msg);
}

template<class IbMessageT>
//[[deprecated]]
void TraceTx(logging::ILogger* logger, ib::mw::EndpointAddress addr, const IbMessageT& msg)
{
    logger->Trace("Send from {}: {}", addr, msg);
}

template<class IbMessageT>
void TraceRx(logging::ILogger* logger, const IServiceId* addr, const IbMessageT& msg)
{
  logger->Trace("Recv from {}: {}", addr->GetServiceId(), msg);
}

template<class IbMessageT>
void TraceTx(logging::ILogger* logger, const IServiceId* addr, const IbMessageT& msg)
{
  logger->Trace("Send from {}: {}", addr->GetServiceId(), msg);
}

// Don't trace LogMessages - this could cause cycles!
//[[deprecated]]
inline void TraceRx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}
//[[deprecated]]
inline void TraceTx(logging::ILogger* /*logger*/, ib::mw::EndpointAddress /*addr*/, const logging::LogMsg& /*msg*/) {}
inline void TraceRx(logging::ILogger* /*logger*/, IServiceId* /*addr*/, const logging::LogMsg& /*msg*/) {}
inline void TraceTx(logging::ILogger* /*logger*/, IServiceId* /*addr*/, const logging::LogMsg& /*msg*/) {}

} // namespace mw
} // namespace ib

