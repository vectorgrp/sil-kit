// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "ILogger.hpp"

#include <algorithm>
#include <string>
#include <unordered_set>

#include <cstdlib>

#include "fmt/format.h"


namespace VSilKit {


template <typename... Args>
void TraceEvent(SilKit::Services::Logging::ILogger *logger, fmt::string_view fileName, size_t line,
                fmt::string_view function, const void *object, Args &&...args)
{
    if (logger == nullptr)
    {
        return;
    }

    if ((logger->GetLogLevel() > SilKit::Services::Logging::Level::Trace))
    {
        return;
    }

    thread_local std::string formattedPrefix;

    formattedPrefix.clear();
    fmt::format_to(std::back_inserter(formattedPrefix), "[{}:{}] [{}", fileName, line, function);

    if (object)
    {
        fmt::format_to(std::back_inserter(formattedPrefix), " {}", object);
    }

    formattedPrefix.push_back(']');

    thread_local std::string formattedArguments;

    formattedArguments.clear();
    fmt::format_to(std::back_inserter(formattedArguments), std::forward<Args>(args)...);

    SilKit::Services::Logging::Trace(logger, "{} {}", formattedPrefix, formattedArguments);
}


namespace Details {


inline constexpr auto ExtractFileNameFromFileMacro(fmt::string_view s) -> fmt::string_view
{
    if (s.size() == 0)
    {
        return s;
    }

    const char *ptr{s.data() + s.size() - 1};

    while (ptr >= s.data())
    {
        const char ch{*ptr};

        if ((ch == '/') || (ch == '\\'))
        {
            ++ptr;
            break;
        }

        --ptr;
    }

    if (ptr < s.data())
    {
        return s;
    }

    return fmt::string_view{ptr, static_cast<size_t>(s.data() + s.size() - ptr)};
}


} // namespace Details


} // namespace VSilKit


#ifdef SILKIT_ENABLE_TRACING_INSTRUMENTATION

#    define DETAILS_VSILKIT_TRACE_EVENT(logger, object, ...) \
        ::VSilKit::TraceEvent(logger, ::VSilKit::Details::ExtractFileNameFromFileMacro(__FILE__), __LINE__, __func__, \
                              object, __VA_ARGS__)

#else

#    define DETAILS_VSILKIT_TRACE_EVENT(logger, object, ...) \
        do \
        { \
        } while (false)

#endif


#define SILKIT_TRACE_METHOD(logger, ...) DETAILS_VSILKIT_TRACE_EVENT(logger, this, __VA_ARGS__)


#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioAcceptor 0
#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioConnector 0
#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioIoContext 0
#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioGenericRawByteStream 0

#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_ConnectPeer 0

#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioConnection 0
#define SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioPeer 0
