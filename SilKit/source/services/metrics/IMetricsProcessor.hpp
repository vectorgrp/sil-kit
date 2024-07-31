// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace VSilKit {

struct MetricsUpdate;

struct IMetricsProcessor
{
    virtual ~IMetricsProcessor() = default;
    virtual void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) = 0;
};

} // namespace VSilKit