// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsSink.hpp"

#include <fstream>
#include <mutex>
#include <string>

namespace VSilKit {

class MetricsJsonFileSink : public IMetricsSink
{
    std::mutex _mx;
    std::ofstream _ofstream;

public:
    explicit MetricsJsonFileSink(const std::string& path);

    void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) override;
};

} // namespace VSilKit