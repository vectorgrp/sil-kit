// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "LabelMatching.hpp"

namespace {

using namespace testing;

using namespace SilKit::Util;
using namespace SilKit::Services;

class Test_LabelMatching : public ::testing::Test
{
protected:
    Test_LabelMatching() {}
};

TEST_F(Test_LabelMatching, match_single_labels)
{
    std::vector<MatchingLabel> labels1{};
    std::vector<MatchingLabel> labels2{};

    // Matching is symmetric, reversing the labels (if different) should result in the same behavior

    // One side Empty
    {
        // Optional <-> Empty => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
        EXPECT_EQ(MatchLabels(labels2, labels1), true);

        // Mandatory <-> Empty => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        labels2 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);

        // Empty <-> Empty => Match
        labels1 = {};
        labels1 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
    }

    // Same Key+Val
    {
        // Optional <-> Optional => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // Optional <-> Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
        EXPECT_EQ(MatchLabels(labels2, labels1), true);

        // Mandatory <-> Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
    }

    // Same Key, different Val
    {
        // Optional <-> Optional => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValB", MatchingLabel::Kind::Optional}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);

        // Optional <-> Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);

        // Mandatory <-> Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyA", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
    }

    // Different Key
    {
        // Optional <-> Optional => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyB", "ValA", MatchingLabel::Kind::Optional}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // Optional <-> Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyB", "ValA", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);

        // Mandatory <-> Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyB", "ValA", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
    }
}

TEST_F(Test_LabelMatching, match_multiple_labels)
{
    std::vector<MatchingLabel> labels1{};
    std::vector<MatchingLabel> labels2{};

    // Matching is symmetric, reversing the labels (if different) should result in the same behavior

    // One side Empty
    {
        // 2 Optional <-> Empty => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
        EXPECT_EQ(MatchLabels(labels2, labels1), true);

        // 2 Mandatory <-> Empty => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        labels2 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);

        // Mandatory, Optional <-> Empty => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);
    }

    // Same Key+Val
    {
        // 2 Optional <-> 2 Optional => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // 2 Optional <-> 2 Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
        EXPECT_EQ(MatchLabels(labels2, labels1), true);

        // 2 Mandatory <-> 2 Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // Optional, Mandatory <-> Optional, Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // Mandatory, Optional <-> Optional, Mandatory => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);
        EXPECT_EQ(MatchLabels(labels2, labels1), true);
    }

    // Same Key, different Val covered by match_single_labels

    // Different Key
    {
        // 2 Optional <-> 2 Optional => Match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyC", "ValC", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyD", "ValD", MatchingLabel::Kind::Optional}};
        EXPECT_EQ(MatchLabels(labels1, labels2), true);

        // 2 Optional <-> 2 Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Optional},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Optional}};
        labels2 = {MatchingLabel{"KeyC", "ValC", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyD", "ValD", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
        EXPECT_EQ(MatchLabels(labels2, labels1), false);

        // Mandatory <-> Mandatory => No match
        labels1 = {MatchingLabel{"KeyA", "ValA", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyB", "ValB", MatchingLabel::Kind::Mandatory}};
        labels2 = {MatchingLabel{"KeyC", "ValC", MatchingLabel::Kind::Mandatory},
                   MatchingLabel{"KeyD", "ValD", MatchingLabel::Kind::Mandatory}};
        EXPECT_EQ(MatchLabels(labels1, labels2), false);
    }
}

} // anonymous namespace
