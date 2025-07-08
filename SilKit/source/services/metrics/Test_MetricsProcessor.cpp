// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IMetricsSink.hpp"
#include "MetricsDatatypes.hpp"
#include "MetricsProcessor.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace {

using VSilKit::IMetricsSink;
using VSilKit::MetricsProcessor;
using VSilKit::MetricsUpdate;
using VSilKit::MetricData;
using VSilKit::MetricKind;

using testing::ContainerEq;

struct MockMetricsSink : IMetricsSink
{
    MOCK_METHOD(void, Process, (const std::string&, const MetricsUpdate&), (override));
};

MATCHER_P(MetricsUpdateWithMetrics, metrics, "")
{
    return Matches(ContainerEq(metrics))(arg.metrics);
}


TEST(Test_MetricsProcessor, metrics_are_cached_until_sinks_are_set)
{
    const std::string participantName{"Participant Name"};

    MetricsUpdate updateOne;
    updateOne.metrics.emplace_back(MetricData{1, "1", MetricKind::COUNTER, "1"});

    MetricsUpdate updateTwo;
    updateTwo.metrics.emplace_back(MetricData{2, "2", MetricKind::COUNTER, "2"});

    std::vector<MetricData> metrics;
    std::copy(updateOne.metrics.begin(), updateOne.metrics.end(), std::back_inserter(metrics));
    std::copy(updateTwo.metrics.begin(), updateTwo.metrics.end(), std::back_inserter(metrics));

    std::vector<std::unique_ptr<IMetricsSink>> sinks;
    sinks.emplace_back(std::make_unique<MockMetricsSink>());

    auto& sink = dynamic_cast<MockMetricsSink&>(*sinks[0]);

    EXPECT_CALL(sink, Process(participantName, MetricsUpdateWithMetrics(metrics))).Times(1);

    MetricsProcessor processor{participantName};
    processor.Process(participantName, updateOne);
    processor.Process(participantName, updateTwo);
    processor.SetSinks(std::move(sinks));
}


TEST(Test_MetricsProcessor, metrics_are_cached_until_sinks_are_set_per_origin)
{
    const std::string participantName{"Participant Name"};
    const std::string updateOneOrigin{"One"};
    const std::string updateTwoOrigin{"Two"};

    MetricsUpdate updateOneA;
    updateOneA.metrics.emplace_back(MetricData{1, "1A", MetricKind::COUNTER, "1"});

    MetricsUpdate updateOneB;
    updateOneB.metrics.emplace_back(MetricData{2, "1B", MetricKind::COUNTER, "1"});

    MetricsUpdate updateTwoA;
    updateTwoA.metrics.emplace_back(MetricData{2, "2", MetricKind::COUNTER, "2"});

    MetricsUpdate updateTwoB;
    updateTwoB.metrics.emplace_back(MetricData{2, "2", MetricKind::COUNTER, "2"});

    std::vector<MetricData> metricsOne;
    std::copy(updateOneA.metrics.begin(), updateOneA.metrics.end(), std::back_inserter(metricsOne));
    std::copy(updateOneB.metrics.begin(), updateOneB.metrics.end(), std::back_inserter(metricsOne));

    std::vector<MetricData> metricsTwo;
    std::copy(updateTwoA.metrics.begin(), updateTwoA.metrics.end(), std::back_inserter(metricsTwo));
    std::copy(updateTwoB.metrics.begin(), updateTwoB.metrics.end(), std::back_inserter(metricsTwo));

    std::vector<std::unique_ptr<IMetricsSink>> sinks;
    sinks.emplace_back(std::make_unique<MockMetricsSink>());

    auto& sink = dynamic_cast<MockMetricsSink&>(*sinks[0]);

    EXPECT_CALL(sink, Process(updateOneOrigin, MetricsUpdateWithMetrics(metricsOne))).Times(1);
    EXPECT_CALL(sink, Process(updateTwoOrigin, MetricsUpdateWithMetrics(metricsTwo))).Times(1);

    MetricsProcessor processor{participantName};
    processor.Process(updateOneOrigin, updateOneA);
    processor.Process(updateTwoOrigin, updateTwoA);
    processor.Process(updateOneOrigin, updateOneB);
    processor.Process(updateTwoOrigin, updateTwoB);
    processor.SetSinks(std::move(sinks));
}


TEST(Test_MetricsProcessor, metrics_are_processed_directly_after_sinks_are_set)
{
    const std::string participantName{"Participant Name"};

    MetricsUpdate updateOne;
    updateOne.metrics.emplace_back(MetricData{1, "1", MetricKind::COUNTER, "1"});

    MetricsUpdate updateTwo;
    updateTwo.metrics.emplace_back(MetricData{2, "2", MetricKind::COUNTER, "2"});

    std::vector<std::unique_ptr<IMetricsSink>> sinks;
    sinks.emplace_back(std::make_unique<MockMetricsSink>());

    auto& sink = dynamic_cast<MockMetricsSink&>(*sinks[0]);

    EXPECT_CALL(sink, Process(participantName, MetricsUpdateWithMetrics(updateOne.metrics))).Times(1);
    EXPECT_CALL(sink, Process(participantName, MetricsUpdateWithMetrics(updateTwo.metrics))).Times(1);

    MetricsProcessor processor{participantName};
    processor.SetSinks(std::move(sinks));
    processor.Process(participantName, updateOne);
    processor.Process(participantName, updateTwo);
}


TEST(Test_MetricsProcessor, receive_handler_ignores_metrics_from_own_participant)
{
    const std::string simulationName{"Simulation Name"};
    const std::string participantName{"Participant Name"};

    MetricsUpdate updateOne;
    updateOne.metrics.emplace_back(MetricData{1, "1", MetricKind::COUNTER, "1"});

    MetricsUpdate updateTwo;
    updateTwo.metrics.emplace_back(MetricData{2, "2", MetricKind::COUNTER, "2"});

    std::vector<std::unique_ptr<IMetricsSink>> sinks;
    sinks.emplace_back(std::make_unique<MockMetricsSink>());

    auto& sink = dynamic_cast<MockMetricsSink&>(*sinks[0]);

    EXPECT_CALL(sink, Process(participantName, MetricsUpdateWithMetrics(updateOne.metrics))).Times(1);
    EXPECT_CALL(sink, Process(participantName, MetricsUpdateWithMetrics(updateTwo.metrics))).Times(0);

    MetricsProcessor processor{participantName};
    processor.SetSinks(std::move(sinks));
    processor.Process(participantName, updateOne);
    processor.OnMetricsUpdate(simulationName, participantName, updateTwo);
}


} // anonymous namespace