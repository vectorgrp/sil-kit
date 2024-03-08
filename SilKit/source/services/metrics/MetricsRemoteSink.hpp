// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsSink.hpp"

namespace VSilKit {
struct IMetricsSender;
} // namespace VSilKit

namespace VSilKit {

class MetricsRemoteSink : public IMetricsSink
{
    std::string _participantName;
    IMetricsSender* _sender{nullptr};

public:
    explicit MetricsRemoteSink(std::string participantName, IMetricsSender& sender);

    void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) override;
};

} // namespace VSilKit