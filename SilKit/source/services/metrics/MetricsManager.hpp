// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMsgForMetricsSender.hpp"
#include "IMetricsManager.hpp"
#include "Metrics.hpp"

#include "LoggerMessage.hpp"
#include "IServiceEndpoint.hpp"
#include "IParticipantInternal.hpp"
#include "IAttributeMetric.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <limits>


namespace VSilKit {

using MetricClock = std::chrono::steady_clock;
using MetricTimePoint = MetricClock::time_point;


class MetricsManager : public IMetricsManager
{
private:
    struct IMetric
    {
        virtual ~IMetric() = default;
        virtual auto GetMetricKind() const -> MetricKind = 0;
        virtual auto GetUpdateTime() const -> MetricTimePoint = 0;
        virtual auto FormatValue() const -> std::string = 0;
    };

    class CounterMetric;
    class StatisticMetric;
    class StringListMetric;
    class AttributeMetric;

public:
    MetricsManager(std::string participantName, IMetricsProcessor& processor);

    void SetLogger(SilKit::Services::Logging::ILogger& logger);

public: // IMetricsManager
    void SubmitUpdates() override;
    auto GetCounter(MetricName name) -> ICounterMetric* override;
    auto GetStatistic(MetricName name) -> IStatisticMetric* override;
    auto GetStringList(MetricName name) -> IStringListMetric* override;
    auto GetAttribute(MetricName name) -> IAttributeMetric* override;

private:
    auto GetOrCreateMetric(MetricName name, MetricKind kind) -> IMetric*;

private:
    std::string _participantName;
    IMetricsProcessor* _processor{nullptr};
    SilKit::Services::Logging::ILogger* _logger{nullptr};

    // Metrics

    std::mutex _mutex;
    std::unordered_map<std::string, std::unique_ptr<IMetric>> _metrics;
    MetricTimePoint _lastSubmitUpdate;
};


} // namespace VSilKit
