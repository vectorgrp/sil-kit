// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

#include "IMetricsProcessor.hpp"
#include "MetricsDatatypes.hpp"
#include "MetricsManager.hpp"

namespace {

using VSilKit::MetricData;
using VSilKit::MetricKind;
using VSilKit::MetricsManager;
using VSilKit::MetricName;
using VSilKit::ToString;
using VSilKit::MetricsUpdate;
using VSilKit::IMetricsProcessor;

using testing::Contains;
using testing::Matches;

struct MockMetricsProcessor : IMetricsProcessor
{
    MOCK_METHOD(void, Process, (const std::string&, const MetricsUpdate&), (override));
};

MATCHER_P2(MetricDataWithNameAndKind, metricName, metricKind, "")
{
    return arg.name == ToString(metricName) && arg.kind == metricKind;
}

MATCHER_P2(MetricsUpdateWithSingleMetricWithNameAndKind, metricName, metricKind, "")
{
    return arg.metrics.size() == 1 && Matches(MetricDataWithNameAndKind(metricName, metricKind))(arg.metrics.front());
}


TEST(Test_MetricsManager, counter_metric_create_and_update_only_submits_after_change)
{
    const std::string participantName{"Participant Name"};
    const MetricName metricName{"Counter Metric"};

    MetricsUpdate metricsUpdate;
    metricsUpdate.metrics.emplace_back(MetricData{});

    MockMetricsProcessor mockMetricsProcessor;
    EXPECT_CALL(mockMetricsProcessor,
                Process(participantName, MetricsUpdateWithSingleMetricWithNameAndKind(metricName, MetricKind::COUNTER)))
        .Times(1);

    MetricsManager metricsManager{participantName, mockMetricsProcessor};
    // no metrics to report, no Process call should be made
    metricsManager.SubmitUpdates();

    auto metric = metricsManager.GetCounter(metricName);
    // no metric value to report, no Process call should be made
    metricsManager.SubmitUpdates();

    metric->Set(12345);
    // metric value to report, single Process call
    metricsManager.SubmitUpdates();
}


TEST(Test_MetricsManager, statistic_metric_create_and_update_only_submits_after_change)
{
    const std::string participantName{"Participant Name"};
    const MetricName metricName{"Statistic Metric"};

    MetricsUpdate metricsUpdate;
    metricsUpdate.metrics.emplace_back(MetricData{});

    MockMetricsProcessor mockMetricsProcessor;
    EXPECT_CALL(mockMetricsProcessor, Process(participantName, MetricsUpdateWithSingleMetricWithNameAndKind(
                                                                   metricName, MetricKind::STATISTIC)))
        .Times(1);

    MetricsManager metricsManager{participantName, mockMetricsProcessor};
    // no metrics to report, no Process call should be made
    metricsManager.SubmitUpdates();

    auto metric = metricsManager.GetStatistic(metricName);
    // no metric value to report, no Process call should be made
    metricsManager.SubmitUpdates();

    metric->Take(123.456);
    // metric value to report, single Process call
    metricsManager.SubmitUpdates();
}


TEST(Test_MetricsManager, string_list_metric_create_and_update_only_submits_after_change)
{
    const std::string participantName{"Participant Name"};
    const MetricName metricName{"String-List Metric"};

    MetricsUpdate metricsUpdate;
    metricsUpdate.metrics.emplace_back(MetricData{});

    MockMetricsProcessor mockMetricsProcessor;
    EXPECT_CALL(mockMetricsProcessor, Process(participantName, MetricsUpdateWithSingleMetricWithNameAndKind(
                                                                   metricName, MetricKind::STRING_LIST)))
        .Times(1);

    MetricsManager metricsManager{participantName, mockMetricsProcessor};
    // no metrics to report, no Process call should be made
    metricsManager.SubmitUpdates();

    auto metric = metricsManager.GetStringList(metricName);
    // no metric value to report, no Process call should be made
    metricsManager.SubmitUpdates();

    metric->Add("Woweeee!");
    // metric value to report, single Process call
    metricsManager.SubmitUpdates();
}


TEST(Test_MetricsManager, attribute_metric_create_and_update_only_submits_after_change)
{
    const std::string participantName{"Participant Name"};
    const MetricName metricName{"Attribute Metric"};

    MetricsUpdate metricsUpdate;
    metricsUpdate.metrics.emplace_back(MetricData{});

    MockMetricsProcessor mockMetricsProcessor;
    EXPECT_CALL(mockMetricsProcessor, Process(participantName, MetricsUpdateWithSingleMetricWithNameAndKind(
                                                                   metricName, MetricKind::ATTRIBUTE)))
        .Times(1);

    MetricsManager metricsManager{participantName, mockMetricsProcessor};
    // no metrics to report, no Process call should be made
    metricsManager.SubmitUpdates();

    auto metric = metricsManager.GetAttribute(metricName);
    // no metric value to report, no Process call should be made
    metricsManager.SubmitUpdates();

    metric->Add("AttributeValue");
    // metric value to report, single Process call
    metricsManager.SubmitUpdates();
}


} // anonymous namespace
