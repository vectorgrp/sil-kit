// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <chrono>

#include "ICounterMetric.hpp"
#include "IStatisticMetric.hpp"
#include "IStringListMetric.hpp"
#include "IAttributeMetric.hpp"
#include "IMetricsManager.hpp"
#include "IMetricsSender.hpp"
#include "IMetricsProcessor.hpp"


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
