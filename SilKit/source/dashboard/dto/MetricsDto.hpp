// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace SilKit {
namespace Dashboard {

/*! One metric sample: timestamp, participant name, split metric name, and the value.
 *
 *  The three concrete metric kinds differ only in the type of `mv`, so they share this template.
 *  Field order matches what the previous oatpp DTOs emitted (base fields first, then `mv`).
 */
template <typename MetricValueT>
struct MetricDataDto
{
    //! Timestamp.
    int64_t ts{};
    //! Participant name.
    std::string pn;
    //! Metric name, split on '/'.
    std::vector<std::string> mn;
    //! Metric value.
    MetricValueT mv{};
};

using AttributeDataDto = MetricDataDto<std::string>;
using CounterDataDto = MetricDataDto<int64_t>;
using StatisticDataDto = MetricDataDto<std::vector<double>>;

struct MetricsUpdateDto
{
    std::vector<AttributeDataDto> attributes;
    std::vector<CounterDataDto> counters;
    std::vector<StatisticDataDto> statistics;
};

} // namespace Dashboard
} // namespace SilKit
