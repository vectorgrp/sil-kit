// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <chrono>

#include "services/metrics/ICounterMetric.hpp"
#include "services/metrics/IStatisticMetric.hpp"
#include "services/metrics/IStringListMetric.hpp"
#include "services/metrics/IAttributeMetric.hpp"
#include "services/metrics/IMetricsManager.hpp"
#include "services/metrics/IMetricsSender.hpp"
#include "services/metrics/IMetricsProcessor.hpp"


namespace SilKit {
namespace Core {
using VSilKit::ICounterMetric;
using VSilKit::IStatisticMetric;
using VSilKit::IStringListMetric;
using VSilKit::IAttributeMetric;
using VSilKit::IMetricsManager;
using VSilKit::IMetricsProcessor;
using VSilKit::IMetricsSender;
} // namespace Core
} // namespace SilKit
