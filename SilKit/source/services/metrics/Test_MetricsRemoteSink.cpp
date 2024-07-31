// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IMetricsSender.hpp"
#include "MetricsDatatypes.hpp"
#include "MetricsRemoteSink.hpp"

namespace {

using VSilKit::IMetricsSender;
using VSilKit::MetricData;
using VSilKit::MetricKind;
using VSilKit::MetricTimestamp;
using VSilKit::MetricsRemoteSink;
using VSilKit::MetricsUpdate;

using testing::_;

struct MockMetricsSender : IMetricsSender
{
    MOCK_METHOD(void, Send, (const MetricsUpdate&), (override));
};


TEST(Test_MetricsRemoteSink, test_forward_to_sender_only_if_local_origin)
{
    const std::string origin{"Participant Name"};
    const std::string otherOrigin{"Another Participant Name"};

    MetricsUpdate updateOne;
    updateOne.metrics.emplace_back(MetricData{1, "One", MetricKind::COUNTER, "1"});

    MetricsUpdate updateTwo;
    updateTwo.metrics.emplace_back(MetricData{2, "Two", MetricKind::COUNTER, "2"});

    MockMetricsSender sender;

    EXPECT_CALL(sender, Send(updateOne)).Times(1);
    EXPECT_CALL(sender, Send(updateTwo)).Times(0);

    MetricsRemoteSink sink{origin, sender};
    sink.Process(origin, updateOne);
    sink.Process(otherOrigin, updateTwo);
}


} // anonymous namespace
