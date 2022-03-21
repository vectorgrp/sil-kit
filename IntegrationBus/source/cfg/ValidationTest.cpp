// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "Validation.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/util/functional.hpp"

#include "Validation.hpp"

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

// ================================================================================
// TraceSink and Tracer related tests
// ================================================================================

TEST(TestMwCfgValidation, throw_if_usetracesinks_refers_to_unknown_trace_sink)
{
    TraceSink sink;
    sink.name = "Sink1";
    //sink.enabled = true;
    sink.type = TraceSink::Type::PcapFile;

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("UndefinedSink");
    cfg.ethernetControllers.emplace_back(std::move(controller));

    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);
}

TEST(TestMwCfgValidation, throw_if_usetracesinks_refers_to_empty_sink_name)
{
    TraceSink sink;
    sink.name = "Sink1";
    //sink.enabled = true;
    sink.type = TraceSink::Type::PcapFile;

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("");
    cfg.ethernetControllers.emplace_back(std::move(controller));

    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);
}

TEST(TestMwCfgValidation, throw_if_replay_refers_to_unknown_source_name)
{
    TraceSource source;
    source.name = "Source1";
    //source.enabled = true;
    source.type = TraceSource::Type::Mdf4File;
    source.inputPath = "some/file.mf4";

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSources.emplace_back(std::move(source));

    EthernetController controller;
    controller.name = "Eth1";
    controller.replay.useTraceSource = "UnknownSource";

    controller.replay.direction = Replay::Direction::Send;
    cfg.ethernetControllers.emplace_back(std::move(controller));

    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);
}

TEST(TestMwCfgValidation, throw_if_tracesink_has_empty_fields)
{
    TraceSource source;
    source.name = "Source1";
    //source.enabled = true;
    source.type = TraceSource::Type::Undefined;
    source.inputPath = "Foo";

    TraceSink sink;
    sink.name = "Sink1";
    sink.outputPath = "Bar";
    sink.type = TraceSink::Type::Undefined;

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSources.emplace_back(std::move(source));
    cfg.tracing.traceSinks.emplace_back(std::move(sink));

    auto& sourceRef = cfg.tracing.traceSources.at(0);

    sourceRef.inputPath = "";
    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);

    sourceRef.inputPath = "SomeFile";
    sourceRef.name = "";
    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);

    auto& sinkRef = cfg.tracing.traceSinks.at(0);

    sinkRef.outputPath = "";
    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);

    sinkRef.outputPath = "SomeFile";
    sinkRef.name = "";
    EXPECT_THROW(Validate(cfg), ib::ConfigurationError);
}
} // anonymous namespace
