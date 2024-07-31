// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsJsonSink.hpp"

#include "StringHelpers.hpp"

#include <ostream>

namespace {

struct MetricKindString
{
    VSilKit::MetricKind kind;

    friend auto operator<<(std::ostream& ostream, const MetricKindString& self) -> std::ostream&
    {
        switch (self.kind)
        {
        case VSilKit::MetricKind::COUNTER:
            return ostream << "COUNTER";
        case VSilKit::MetricKind::STATISTIC:
            return ostream << "STATISTIC";
        case VSilKit::MetricKind::STRING_LIST:
            return ostream << "STRING_LIST";
        default:
            return ostream << static_cast<std::underlying_type_t<VSilKit::MetricKind>>(self.kind);
        }
    }
};

} // namespace

namespace VSilKit {

MetricsJsonSink::MetricsJsonSink(std::unique_ptr<std::ostream> ostream)
    : _ostream{std::move(ostream)}
{
}

void MetricsJsonSink::Process(const std::string& origin, const MetricsUpdate& metricsUpdate)
{
    std::lock_guard<decltype(_mx)> lock{_mx};

    for (const auto& data : metricsUpdate.metrics)
    {
        *_ostream << R"({"ts":)" << data.timestamp << R"(,"pn":")" << SilKit::Util::EscapedJsonString{origin}
                  << R"(","mn":")" << SilKit::Util::EscapedJsonString{data.name} << R"(","mk":")"
                  << MetricKindString{data.kind} << R"(","mv":)" << data.value << R"(})" << '\n';
    }

    *_ostream << std::flush;
}

} // namespace VSilKit
