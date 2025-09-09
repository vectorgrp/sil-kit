// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "LoggerMessage.hpp"
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
    if (logger->GetLogLevel() == Logging::Level::Trace)
    {
        Logging::LoggerMessage lm{logger, Logging::Level::Trace};
        lm.SetMessage("Recv message");
        lm.SetKeyValue(addr->GetServiceDescriptor());
        lm.SetKeyValue(Logging::Keys::from, from.GetParticipantName());
        lm.FormatKeyValue(Logging::Keys::msg, "{}", msg);

        auto virtualTimeStamp = GetTimestamp(msg);
        if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
        {
            lm.FormatKeyValue(Logging::Keys::virtualTimeNS, "{}", virtualTimeStamp.count());
        }
    lm.Dispatch();
    }
}

template <class SilKitMessageT>
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
    if (logger->GetLogLevel() == Logging::Level::Trace)
    {
        Logging::LoggerMessage lm{logger, Logging::Level::Trace};
        lm.SetMessage("Send message");
        lm.SetKeyValue(addr->GetServiceDescriptor());
        lm.FormatKeyValue(Logging::Keys::msg, "{}", msg);

        auto virtualTimeStamp = GetTimestamp(msg);
        if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
        {
                lm.FormatKeyValue(Logging::Keys::virtualTimeNS, "{}", virtualTimeStamp.count());
        }
        lm.Dispatch();
    }
}

// targeted messages
template <class SilKitMessageT>
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const std::string_view target, const SilKitMessageT& msg)
{
    if (logger->GetLogLevel() == Logging::Level::Trace)
    {
        Logging::LoggerMessage lm{logger, Logging::Level::Trace};
        lm.SetMessage("Send message");
        lm.SetKeyValue(addr->GetServiceDescriptor());
        lm.FormatKeyValue(Logging::Keys::msg, "{}", msg);
        lm.FormatKeyValue(Logging::Keys::to, "{}", target);

        auto virtualTimeStamp = GetTimestamp(msg);
        if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
        {
                lm.FormatKeyValue(Logging::Keys::virtualTimeNS, "{}", virtualTimeStamp.count());
        }
        lm.Dispatch();
    }
}
// Don't trace LogMessages - this could cause cycles!
inline void TraceRx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}
inline void TraceTx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, const Logging::LogMsg& /*msg*/) {}

inline void TraceRx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, Logging::LogMsg&& /*msg*/) {}
inline void TraceTx(Logging::ILoggerInternal* /*logger*/, Core::IServiceEndpoint* /*addr*/, Logging::LogMsg&& /*msg*/) {}
} // namespace Services
} // namespace SilKit
