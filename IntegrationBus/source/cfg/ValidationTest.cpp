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

////////////////////////////////////////
// TraceSink and Tracer related tests:
TEST(TestMwCfgValidation, throw_if_usetracesinks_refers_to_unknown_trace_sink)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Loose;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;

    TraceSink sink;
    sink.name = "Sink1";
    sink.enabled = true;
    sink.type = TraceSink::Type::PcapFile;

    Participant participantConfig;
    participantConfig.name = "P1";
    participantConfig.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("UndefinedSink");
    participantConfig.ethernetControllers.emplace_back(std::move(controller));

    ibConfig.simulationSetup.participants.emplace_back(std::move(participantConfig));

    EXPECT_THROW(Validate(ibConfig.simulationSetup, ibConfig), Misconfiguration);
}

TEST(TestMwCfgValidation, throw_if_usetracesinks_refers_to_empty_sink_name)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Loose;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;

    TraceSink sink;
    sink.name = "Sink1";
    sink.enabled = true;
    sink.type = TraceSink::Type::PcapFile;

    Participant participantConfig;
    participantConfig.name = "P1";
    participantConfig.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("");
    participantConfig.ethernetControllers.emplace_back(std::move(controller));

    ibConfig.simulationSetup.participants.emplace_back(std::move(participantConfig));

    EXPECT_THROW(Validate(ibConfig.simulationSetup, ibConfig), Misconfiguration);
}

TEST(TestMwCfgValidation, throw_if_replay_refers_to_empty_source_name)
{
    Config ibConfig;
    ibConfig.simulationSetup.timeSync.syncPolicy = TimeSync::SyncPolicy::Loose;
    ibConfig.simulationSetup.timeSync.tickPeriod = 0ns;

    TraceSource source;
    source.name = "Source1";
    source.enabled = true;
    source.type = TraceSource::Type::Mdf4File;
    source.inputPath = "some/file.mf4";

    Participant participantConfig;
    participantConfig.name = "P1";
    participantConfig.traceSources.emplace_back(std::move(source));

    ReplayConfig replayConfig;
    replayConfig.direction = ReplayConfig::Direction::Send;
    EthernetController controller;
    controller.name = "Eth1";
    controller.replay.useTraceSource = "";

    controller.replay.withReplayConfigs.emplace_back(std::move(replayConfig));
    participantConfig.ethernetControllers.emplace_back(std::move(controller));
    ibConfig.simulationSetup.participants.emplace_back(std::move(participantConfig));

    EXPECT_THROW(Validate(ibConfig.simulationSetup, ibConfig), Misconfiguration);
}

} // anonymous namespace
