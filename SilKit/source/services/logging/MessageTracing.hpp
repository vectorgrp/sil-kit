// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "LoggerMessage.hpp"
#include "IServiceEndpoint.hpp"
#include "ServiceDescriptor.hpp"
#include "traits/SilKitMsgTraits.hpp"

#include "YamlParser.hpp"


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

namespace Detail {
template <class SilKitMessageT>
void TraceMessageCommon(Logging::ILoggerInternal* logger,
                       const char* messageString,
                       const Core::IServiceEndpoint* addr,
                       const SilKitMessageT& msg,
                       std::string_view keyString = {},
                       std::string_view valueString = {})
{

    if constexpr (std::is_same_v<SilKitMessageT, SilKit::Services::Logging::LogMsg>)
    {
        // Don't trace LogMessages - this could cause cycles!
        return;
    }
    else
    {
        if (logger->GetLogLevel() == Logging::Level::Trace)
        {
            Logging::LoggerMessage lm{logger, Logging::Level::Trace};
            lm.SetMessage(messageString);
            lm.SetKeyValue(addr->GetServiceDescriptor());
            lm.FormatKeyValue(Logging::Keys::msg, "{}", msg);

            if (!keyString.empty() && ! valueString.empty())
            {
                lm.SetKeyValue(keyString, valueString);
            }

            auto virtualTimeStamp = GetTimestamp(msg);
            if (virtualTimeStamp != std::chrono::nanoseconds::duration::min())
            {
                lm.FormatKeyValue(Logging::Keys::virtualTimeNS, "{}", virtualTimeStamp.count());
            }
            // Turn the Raw-logging into a trait when we have enough types that implement it
            if constexpr (std::is_same_v<SilKitMessageT, SilKit::Services::Flexray::FlexrayControllerConfig>)
            {
                lm.SetKeyValue(Logging::Keys::raw, SilKit::Config::SerializeAsJson(msg));
            }
            lm.Dispatch();
        }
    }
}
} // namespace Detail
template <class SilKitMessageT>
void TraceRx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg,
             const Core::ServiceDescriptor& from)
{
    Detail::TraceMessageCommon(logger, "Recv message", addr, msg, Logging::Keys::from, from.GetParticipantName());
}

template <class SilKitMessageT>
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const SilKitMessageT& msg)
{
    Detail::TraceMessageCommon(logger, "Send message", addr, msg);
}

template <class SilKitMessageT>
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const std::string_view target, const SilKitMessageT& msg)
{
    Detail::TraceMessageCommon(logger, "Send targetted message", addr, msg, Logging::Keys::to, target);
}
} // namespace Services
} // namespace SilKit
