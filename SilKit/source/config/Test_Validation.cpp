/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "Validation.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "functional.hpp"

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

using namespace SilKit::Config;

using namespace std::chrono_literals;

// ================================================================================
// TraceSink and Tracer related tests
// ================================================================================

TEST(Test_Validation, throw_if_usetracesinks_refers_to_unknown_trace_sink)
{
    TraceSink sink;
    sink.name = "Sink1";
    sink.type = TraceSink::Type::PcapFile;

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("UndefinedSink");
    cfg.ethernetControllers.emplace_back(std::move(controller));

    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);
}

TEST(Test_Validation, throw_if_usetracesinks_refers_to_empty_sink_name)
{
    TraceSink sink;
    sink.name = "Sink1";
    sink.type = TraceSink::Type::PcapFile;

    ParticipantConfiguration cfg;
    cfg.participantName = "P1";
    cfg.tracing.traceSinks.emplace_back(std::move(sink));

    EthernetController controller;
    controller.name = "Eth1";
    controller.useTraceSinks.push_back("");
    cfg.ethernetControllers.emplace_back(std::move(controller));

    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);
}

TEST(Test_Validation, throw_if_replay_refers_to_unknown_source_name)
{
    TraceSource source;
    source.name = "Source1";
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

    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);
}

TEST(Test_Validation, throw_if_tracesink_has_empty_fields)
{
    TraceSource source;
    source.name = "Source1";
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
    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);

    sourceRef.inputPath = "SomeFile";
    sourceRef.name = "";
    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);

    auto& sinkRef = cfg.tracing.traceSinks.at(0);

    sinkRef.outputPath = "";
    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);

    sinkRef.outputPath = "SomeFile";
    sinkRef.name = "";
    EXPECT_THROW(Validate(cfg), SilKit::ConfigurationError);
}

} // anonymous namespace
