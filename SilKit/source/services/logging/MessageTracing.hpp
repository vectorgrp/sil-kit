// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/logging/ILogger.hpp"
#include "../internal/IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {

template<class SilKitMessageT>
void TraceRx(Logging::ILogger* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
  logger->Trace("Recv from {}: {}", addr->GetServiceDescriptor(), msg);
}

template<class SilKitMessageT>
void TraceTx(Logging::ILogger* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
  logger->Trace("Send from {}: {}", addr->GetServiceDescriptor(), msg);
}

// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(Logging::ILogger* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}
inline void TraceTx(Logging::ILogger* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}

} // namespace Services
} // namespace SilKit

