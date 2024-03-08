// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsDatatypes.hpp"

#include <ostream>
#include <type_traits>


namespace VSilKit {


auto operator<<(std::ostream& os, const MetricKind& metricKind) -> std::ostream&
{
    switch (metricKind)
    {
    case MetricKind::COUNTER:
        return os << "MetricKind::COUNTER";
    case MetricKind::STATISTIC:
        return os << "MetricKind::STATISTIC";
    case MetricKind::STRING_LIST:
        return os << "MetricKind::STRING_LIST";
    default:
        return os << "MetricKind(" << static_cast<std::underlying_type_t<MetricKind>>(metricKind) << ")";
    }
}

auto operator<<(std::ostream& os, const MetricInfo& metricInfo) -> std::ostream&
{
    return os << "MetricInfo{name=" << metricInfo.name << ", id=" << metricInfo.id << "}";
}

auto operator<<(std::ostream& os, const MetricsRegistration& metricsRegistration) -> std::ostream&
{
    os << "MetricsRegistration{metrics=[";

    bool first{true};
    for (const auto& metricInfo : metricsRegistration.metrics)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ", ";
        }

        os << metricInfo;
    }

    os << "]}";
    return os;
}

auto operator<<(std::ostream& os, const MetricData& metricData) -> std::ostream&
{
    return os << "MetricData{timestamp=" << metricData.timestamp << ", name=" << metricData.name
              << ", kind=" << metricData.kind << ", value=" << metricData.value << "}";
}

auto operator<<(std::ostream& os, const MetricsUpdate& metricsUpdate) -> std::ostream&
{
    os << "MetricsUpdate{metrics=[";

    bool first{true};
    for (const auto& metricData : metricsUpdate.metrics)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << ", ";
        }

        os << metricData;
    }

    os << "]}";
    return os;
}


} // namespace VSilKit
