// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <initializer_list>

namespace VSilKit {

struct ICounterMetric;
struct IStatisticMetric;
struct IStringListMetric;
struct IAttributeMetric;

using MetricName = std::initializer_list<std::string>;

inline auto ToString(MetricName stringList) -> std::string
{
    std::stringstream ss;
    if(stringList.size() == 1)
    {
        return *stringList.begin();
    }

    for (auto&& s : stringList)
    {
        ss << s << "/";
    }
    return ss.str();
}

struct IMetricsManager
{
    virtual ~IMetricsManager() = default;
    virtual void SubmitUpdates() = 0;
    virtual auto GetCounter(MetricName name) -> ICounterMetric* = 0;
    virtual auto GetStatistic(MetricName name) -> IStatisticMetric* = 0;
    virtual auto GetStringList(MetricName name) -> IStringListMetric* = 0;
    virtual auto GetAttribute(MetricName name) -> IAttributeMetric* = 0;
};

} // namespace VSilKit