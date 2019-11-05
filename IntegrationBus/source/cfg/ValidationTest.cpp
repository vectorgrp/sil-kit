// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Validation.hpp"


#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

namespace {

using namespace std::chrono_literals;
using namespace std::placeholders;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

using namespace ib::mw;
using namespace ib::cfg;

using namespace std::chrono_literals;

TEST(TestMwCfgValidation, throw_if_tickperiod_is_unconfigured_when_using_strictsync)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Strict;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;
    EXPECT_THROW(Validate(ibConfig.simulationSetup.timeSync, ibConfig), Misconfiguration);
}

TEST(TestMwCfgValidation, throw_if_tickperiod_is_unconfigured_when_using_DiscreteTime_sync)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Loose;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;

    Participant participantConfig;
    ParticipantController controller;
    controller.syncType = SyncType::DiscreteTime;
    participantConfig.participantController = controller;

    ibConfig.simulationSetup.participants.emplace_back(std::move(participantConfig));

    EXPECT_THROW(Validate(ibConfig.simulationSetup.timeSync, ibConfig), Misconfiguration);
}

TEST(TestMwCfgValidation, throw_if_tickperiod_is_unconfigured_when_using_DiscreteTimePassive_sync)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Loose;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;

    Participant participantConfig;
    ParticipantController controller;
    controller.syncType = SyncType::DiscreteTimePassive;
    participantConfig.participantController = controller;

    ibConfig.simulationSetup.participants.emplace_back(std::move(participantConfig));

    EXPECT_THROW(Validate(ibConfig.simulationSetup.timeSync, ibConfig), Misconfiguration);
}

} // anonymous namespace
