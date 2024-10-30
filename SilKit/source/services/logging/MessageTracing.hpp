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

#include "ILoggerInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "ServiceDescriptor.hpp"
#include "traits/SilKitMsgTraits.hpp"


namespace SilKit {
namespace Services {


template <typename MsgT>
std::chrono::nanoseconds GetTimestamp(MsgT& msg,
                                      std::enable_if_t<Core::HasTimestamp<MsgT>::value, bool> = true)
{
    return msg.timestamp;
}

template <typename MsgT>
std::chrono::nanoseconds GetTimestamp(MsgT& /*msg*/,
                  std::enable_if_t<!Core::HasTimestamp<MsgT>::value, bool> = false)
{
    return std::chrono::nanoseconds::duration::min();
}




template <class SilKitMessageT>
void TraceRx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg,

             const Core::ServiceDescriptor& from)
{
    Logging::LoggerMessage lm{logger, Logging::Level::Trace};
    lm.SetMessage("Recv message");

    for (const auto& pair : addr->GetServiceDescriptor().to_keyValues())
    {
        lm.SetKeyValue(pair.first, pair.second);
    }
    lm.SetKeyValue(Logging::Keys::from, from.GetParticipantName());
    lm.SetKeyValue(Logging::Keys::msg, (fmt::format("{}", msg)));

    auto virtualTimeStamp = GetTimestamp(msg);
    if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
    {
        lm.SetKeyValue(Logging::Keys::virtualTimeNS, (fmt::format("{}", virtualTimeStamp.count())));
    }

    lm.Dispatch();
}

template <class SilKitMessageT>
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
    Logging::LoggerMessage lm{logger, Logging::Level::Trace};
    lm.SetMessage("Send message");
   
    for (const auto& pair : addr->GetServiceDescriptor().to_keyValues())
    {
        lm.SetKeyValue(pair.first, pair.second);
    }

    lm.SetKeyValue(Logging::Keys::msg, (fmt::format("{}", msg)));

    auto virtualTimeStamp = GetTimestamp(msg);
    if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
    {
        lm.SetKeyValue(Logging::Keys::virtualTimeNS, (fmt::format("{}", virtualTimeStamp.count())));
    }
    lm.Dispatch();
}

// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}
inline void TraceTx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}

inline void TraceRx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, Logging::LogMsg&& /*msg*/) {}
inline void TraceTx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, Logging::LogMsg&& /*msg*/) {}
} // namespace Services
} // namespace SilKit
