// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Validation.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/sim/fr/FrDatatypes.hpp"
#include "ib/cfg/Config.hpp"

namespace {

using namespace ib::sim::fr;

TEST(TestSimFrValidation, throw_if_gColdstartAttempts_is_out_of_range)
{
    ClusterParameters clusterParams;
    clusterParams.gColdstartAttempts = 0;
    EXPECT_THROW(Validate(clusterParams), ib::cfg::Misconfiguration);
}

TEST(TestSimFrValidation, throw_if_pAllowHaltDueToClock_is_not_0_or_1)
{
    NodeParameters nodeParams;
    nodeParams.pAllowHaltDueToClock = 2;
    EXPECT_THROW(Validate(nodeParams), ib::cfg::Misconfiguration);
}

} // anonymous namespace
