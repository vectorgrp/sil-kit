// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include <iosfwd>
#include <string>
#include <vector>
#include <initializer_list>
#include <string_view>


namespace VSilKit {


using MetricId = uint64_t;
using MetricTimestamp = uint64_t;


enum struct MetricKind
{
    COUNTER,
    STATISTIC,
    STRING_LIST,
    ATTRIBUTE,
};


struct MetricData
{
    MetricTimestamp timestamp;
    std::string name;
    MetricKind kind;
    std::string value;
};


struct MetricsUpdate
{
    std::vector<MetricData> metrics;
};

using MetricName = std::initializer_list<std::string_view>;
auto ToString(MetricName stringList) -> std::string;


auto operator==(const MetricData& lhs, const MetricData& rhs) -> bool;

auto operator==(const MetricsUpdate& lhs, const MetricsUpdate& rhs) -> bool;


auto operator<<(std::ostream& os, const MetricKind& metricKind) -> std::ostream&;

auto operator<<(std::ostream& os, const MetricData& metricData) -> std::ostream&;

auto operator<<(std::ostream& os, const MetricsUpdate& metricValue) -> std::ostream&;


} // namespace VSilKit
