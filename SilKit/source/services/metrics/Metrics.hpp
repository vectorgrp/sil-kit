// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <chrono>

#include "ICounterMetric.hpp"
#include "IStatisticMetric.hpp"
#include "IStringListMetric.hpp"
#include "IMetricsManager.hpp"
#include "IMetricsSender.hpp"
#include "IMetricsProcessor.hpp"


namespace VSilKit {


using MetricClock = std::chrono::steady_clock;
using MetricTimePoint = MetricClock::time_point;


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ICounterMetric;
using VSilKit::IStatisticMetric;
using VSilKit::IStringListMetric;
using VSilKit::IMetricsManager;
using VSilKit::IMetricsProcessor;
using VSilKit::IMetricsSender;
} // namespace Core
} // namespace SilKit
