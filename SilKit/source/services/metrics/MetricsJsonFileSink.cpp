// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsJsonFileSink.hpp"

#include <ostream>

namespace {

struct JsonString
{
    const std::string& string;

    friend auto operator<<(std::ostream& ostream, const JsonString& self) -> std::ostream&
    {
        for (const char ch : self.string)
        {
            switch (ch)
            {
            case '\\':
                ostream.put('\\');
                ostream.put('\\');
                break;
            case '"':
                ostream.put('\\');
                ostream.put('"');
                break;
            default:
                ostream.put(ch);
                break;
            }
        }
        return ostream;
    }
};

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

// NOTE: Since storing the origin as a string is quite wasteful, map it uniquely to an integer and store that.
//       Similar to how the MetricsManager generates the metric IDs.

namespace VSilKit {

MetricsJsonFileSink::MetricsJsonFileSink(const std::string& path)
    : _ofstream{path}
{
}

void MetricsJsonFileSink::Process(const std::string& origin, const MetricsUpdate& metricsUpdate)
{
    std::lock_guard<decltype(_mx)> lock{_mx};

    for (const auto& data : metricsUpdate.metrics)
    {
        _ofstream << R"({"ts":)" << data.timestamp << R"(,"pn":")" << JsonString{origin} << R"(","mn":")"
                  << JsonString{data.name} << R"(","mk":")" << MetricKindString{data.kind} << R"(","mv":)" << data.value
                  << R"(})" << '\n';
    }

    _ofstream << std::flush;
}

} // namespace VSilKit
