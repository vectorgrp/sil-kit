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

namespace Detail {
template <class SilKitMessageT>
void TraceMessageCommon(Logging::ILoggerInternal* logger, const char* messageString, const Core::IServiceEndpoint* addr,
                        const SilKitMessageT& msg, std::string_view keyString = {}, std::string_view valueString = {})
{
    if constexpr (std::is_same_v<SilKitMessageT, SilKit::Services::Logging::LogMsg>)
    {
        // Don't trace LogMessages - this could cause cycles!
        SILKIT_UNUSED_ARG(logger);
        SILKIT_UNUSED_ARG(messageString);
        SILKIT_UNUSED_ARG(addr);
        SILKIT_UNUSED_ARG(msg);
        SILKIT_UNUSED_ARG(keyString);
        SILKIT_UNUSED_ARG(valueString);
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

            if (!keyString.empty() && !valueString.empty())
            {
                lm.SetKeyValue(keyString, valueString);
            }

            if constexpr (Core::HasTimestamp<SilKitMessageT>::value)
            {
                lm.FormatKeyValue(Logging::Keys::virtualTimeNS, "{}", msg.timestamp.count());
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
void TraceTx(Logging::ILoggerInternal* logger, const Core::IServiceEndpoint* addr, const std::string_view target,
             const SilKitMessageT& msg)
{
    Detail::TraceMessageCommon(logger, "Send targetted message", addr, msg, Logging::Keys::to, target);
}

} // namespace Services
} // namespace SilKit
