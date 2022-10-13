/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "ILogger.hpp"
#include "IServiceEndpoint.hpp"
#include "ServiceDescriptor.hpp"

namespace SilKit {
namespace Services {

template<class SilKitMessageT>
void TraceRx(Logging::ILogger* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg,
             const Core::ServiceDescriptor& from)
{
    Logging::Trace(logger, "Recv on {} from {}: {}", addr->GetServiceDescriptor(), from.GetParticipantName(), msg);
}

template<class SilKitMessageT>
void TraceTx(Logging::ILogger* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
  Logging::Trace(logger, "Send from {}: {}", addr->GetServiceDescriptor(), msg);
}

// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(Logging::ILogger* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}
inline void TraceTx(Logging::ILogger* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}

} // namespace Services
} // namespace SilKit

