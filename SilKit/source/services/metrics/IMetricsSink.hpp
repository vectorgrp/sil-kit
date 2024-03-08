// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/util/Span.hpp"

#include "MetricsDatatypes.hpp"

#include <string>

namespace VSilKit {

struct IMetricsSink
{
    virtual ~IMetricsSink() = default;

    virtual void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) = 0;
};

} // namespace VSilKit
