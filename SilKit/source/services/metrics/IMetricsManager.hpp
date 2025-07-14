// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "MetricsDatatypes.hpp"

namespace VSilKit {

struct ICounterMetric;
struct IStatisticMetric;
struct IStringListMetric;
struct IAttributeMetric;


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