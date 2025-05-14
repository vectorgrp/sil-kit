// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MetricsDatatypes.hpp"
#include "MetricsJsonSink.hpp"
#include "StringHelpers.hpp"

#include <algorithm>
#include <sstream>

#include "fmt/format.h"

#include "YamlParser.hpp"

namespace {

using VSilKit::MetricData;
using VSilKit::MetricKind;
using VSilKit::MetricTimestamp;
using VSilKit::MetricsJsonSink;
using VSilKit::MetricsUpdate;

using SilKit::Util::EscapeString;


TEST(Test_MetricsJsonSink, test_json_escaping_and_structure)
{
    const std::string origin{"Participant Name"};

    const MetricTimestamp ts1{1};
    const std::string mn1{"Metric\rName\n1"};
    const auto mk1 = MetricKind::COUNTER;
    const std::string mv1{"1"};

    const MetricTimestamp ts2{2};
    const std::string mn2{"Metric\rName\n2"};
    const auto mk2 = MetricKind::STATISTIC;
    const std::string mv2{"[1,2,3,4]"};

    const MetricTimestamp ts3{3};
    const std::string mn3{"Metric\rName\n3"};
    const auto mk3 = MetricKind::STRING_LIST;
    const std::string mv3a{"A\r\nB\nC"};
    const std::string mv3b{"D\tE\nF\tG"};
    const std::string mv3{fmt::format(R"(["{}","{}"])", EscapeString(mv3a), EscapeString(mv3b))};

    MetricsUpdate update;
    update.metrics.emplace_back(MetricData{ts1, mn1, mk1, mv1});
    update.metrics.emplace_back(MetricData{ts2, mn2, mk2, mv2});
    update.metrics.emplace_back(MetricData{ts3, mn3, mk3, mv3});

    auto ownedOstream = std::make_unique<std::ostringstream>();
    auto& ostream = *ownedOstream;

    // create the sink and process the update

    MetricsJsonSink sink{std::move(ownedOstream)};
    sink.Process(origin, update);

    // extract the resulting json from the string-stream

    auto result = ostream.str();

    // split the result into lines and parse them

    //std::vector<YAML::Node> nodes;

    std::string::size_type pos;
    while ((pos = result.find('\n')) != std::string::npos)
    {
        auto line = result.substr(0, pos);
        result = result.substr(pos + 1);

     //   nodes.emplace_back(YAML::Load(line));
    }

    // checks

    /*
    ASSERT_EQ(nodes.size(), 3);

    ASSERT_TRUE(nodes[0].IsMap());
    ASSERT_EQ(nodes[0]["ts"].as<MetricTimestamp>(), ts1);
    ASSERT_EQ(nodes[0]["mn"].as<std::string>(), mn1);
    ASSERT_EQ(nodes[0]["mk"].as<std::string>(), "COUNTER");
    ASSERT_EQ(nodes[0]["mv"].as<std::string>(), mv1);

    ASSERT_TRUE(nodes[1].IsMap());
    ASSERT_EQ(nodes[1]["ts"].as<MetricTimestamp>(), ts2);
    ASSERT_EQ(nodes[1]["mn"].as<std::string>(), mn2);
    ASSERT_EQ(nodes[1]["mk"].as<std::string>(), "STATISTIC");
    ASSERT_EQ(nodes[1]["mv"].as<std::vector<int>>(), (std::vector<int>{1, 2, 3, 4}));

    ASSERT_TRUE(nodes[2].IsMap());
    ASSERT_EQ(nodes[2]["ts"].as<MetricTimestamp>(), ts3);
    ASSERT_EQ(nodes[2]["mn"].as<std::string>(), mn3);
    ASSERT_EQ(nodes[2]["mk"].as<std::string>(), "STRING_LIST");
    ASSERT_EQ(nodes[2]["mv"].as<std::vector<std::string>>(), (std::vector<std::string>{mv3a, mv3b}));
    */
}


} // anonymous namespace
