// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsSink.hpp"

#include <memory>
#include <mutex>
#include <ostream>
#include <string>

namespace VSilKit {

class MetricsJsonSink : public IMetricsSink
{
    std::mutex _mx;
    std::unique_ptr<std::ostream> _ostream;

public:
    explicit MetricsJsonSink(std::unique_ptr<std::ostream> ostream);

    void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) override;
};

} // namespace VSilKit