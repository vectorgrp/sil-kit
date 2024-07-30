// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include <iosfwd>
#include <string>
#include <vector>


namespace VSilKit {


using MetricId = uint64_t;
using MetricTimestamp = uint64_t;


enum struct MetricKind
{
    COUNTER,
    STATISTIC,
    STRING_LIST,
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


auto operator==(const MetricData& lhs, const MetricData& rhs) -> bool;


auto operator<<(std::ostream& os, const MetricKind& metricKind) -> std::ostream&;

auto operator<<(std::ostream& os, const MetricData& metricData) -> std::ostream&;

auto operator<<(std::ostream& os, const MetricsUpdate& metricValue) -> std::ostream&;


} // namespace VSilKit
