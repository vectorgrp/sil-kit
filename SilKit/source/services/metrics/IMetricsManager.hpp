// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace VSilKit {

struct ICounterMetric;
struct IStatisticMetric;
struct IStringListMetric;
struct IAttributeMetric;

struct IMetricsManager
{
    virtual ~IMetricsManager() = default;
    virtual void SubmitUpdates() = 0;
    virtual auto GetCounter(const std::string& name) -> ICounterMetric* = 0;
    virtual auto GetStatistic(const std::string& name) -> IStatisticMetric* = 0;
    virtual auto GetStringList(const std::string& name) -> IStringListMetric* = 0;
    virtual auto GetAttribute(const std::string& name) -> IAttributeMetric* = 0;
};

} // namespace VSilKit