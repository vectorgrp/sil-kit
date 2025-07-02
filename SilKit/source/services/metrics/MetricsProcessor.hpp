// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsSink.hpp"
#include "MetricsDatatypes.hpp"
#include "MetricsReceiver.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace VSilKit {

class MetricsProcessor
    : public IMetricsProcessor
    , public IMetricsReceiverListener
{
    std::mutex _mutex;

    std::string _participantName;
    SilKit::Services::Logging::ILogger* _logger{nullptr};

    std::atomic<bool> _sinksSetUp{false};
    std::vector<std::unique_ptr<IMetricsSink>> _sinks;

    std::unordered_map<std::string, MetricsUpdate> _updateCache;

public:
    explicit MetricsProcessor(std::string participantName);

public:
    void SetLogger(SilKit::Services::Logging::ILogger& logger);
    void SetSinks(std::vector<std::unique_ptr<IMetricsSink>> sinks);

public: // IMetricsProcessor
    void Process(const std::string& origin, const MetricsUpdate& metricsUpdate) override;

public: // IMetricsReceiverListener
    void OnMetricsUpdate(const std::string& simulationName, const std::string& participantName,
                         const MetricsUpdate& metricsUpdate) override;
};

} // namespace VSilKit
