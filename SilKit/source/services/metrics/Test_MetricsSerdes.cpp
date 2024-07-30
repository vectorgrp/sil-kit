// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MetricsDatatypes.hpp"
#include "MetricsSerdes.hpp"
#include "MessageBuffer.hpp"

namespace {

using VSilKit::MetricsUpdate;
using VSilKit::MetricData;
using VSilKit::MetricKind;

using testing::ContainerEq;


TEST(Test_MetricsSerdes, serialize_deserialize_roundtrip)
{
    MetricsUpdate original;
    original.metrics.emplace_back(MetricData{1, "1", MetricKind::COUNTER, "1"});
    original.metrics.emplace_back(MetricData{2, "2", MetricKind::STATISTIC, "[1,2,3,4]"});
    original.metrics.emplace_back(MetricData{3, "3", MetricKind::STRING_LIST, R"(["1","2","3","4"])"});

    SilKit::Core::MessageBuffer buffer;
    VSilKit::Serialize(buffer, original);

    MetricsUpdate copy;
    VSilKit::Deserialize(buffer, copy);

    ASSERT_THAT(copy.metrics, ContainerEq(original.metrics));
}


} // anonymous namespace