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

#pragma once

#include <chrono>
#include <string>

#include "ParticipantConfiguration.hpp"

#include "yaml-cpp/yaml.h"

#include "SilKitYamlHelper.hpp"

// YAML-cpp serialization/deserialization for ParticipantConfiguration data types
namespace YAML {

using namespace SilKit::Config;

DEFINE_SILKIT_CONVERT(std::chrono::milliseconds);
DEFINE_SILKIT_CONVERT(std::chrono::nanoseconds);

DEFINE_SILKIT_CONVERT(Logging);
DEFINE_SILKIT_CONVERT(Sink);
DEFINE_SILKIT_CONVERT(Sink::Type);
DEFINE_SILKIT_CONVERT(Sink::Format);
DEFINE_SILKIT_CONVERT(SilKit::Services::Logging::Level);

DEFINE_SILKIT_CONVERT(MdfChannel);
DEFINE_SILKIT_CONVERT(Replay);
DEFINE_SILKIT_CONVERT(Replay::Direction);

DEFINE_SILKIT_CONVERT(CanController);

DEFINE_SILKIT_CONVERT(LinController);

DEFINE_SILKIT_CONVERT(EthernetController);

DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayClusterParameters);
DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayNodeParameters);
DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayTxBufferConfig);
DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayChannel);
DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayClockPeriod);
DEFINE_SILKIT_CONVERT(SilKit::Services::Flexray::FlexrayTransmissionMode);
DEFINE_SILKIT_CONVERT(FlexrayController);

DEFINE_SILKIT_CONVERT(SilKit::Services::MatchingLabel::Kind);
DEFINE_SILKIT_CONVERT(SilKit::Services::MatchingLabel);
DEFINE_SILKIT_CONVERT(DataPublisher);
DEFINE_SILKIT_CONVERT(DataSubscriber);
DEFINE_SILKIT_CONVERT(RpcServer);
DEFINE_SILKIT_CONVERT(RpcClient);

DEFINE_SILKIT_CONVERT(HealthCheck);

DEFINE_SILKIT_CONVERT(Tracing);
DEFINE_SILKIT_CONVERT(TraceSink);
DEFINE_SILKIT_CONVERT(TraceSink::Type);
DEFINE_SILKIT_CONVERT(TraceSource);
DEFINE_SILKIT_CONVERT(TraceSource::Type);

DEFINE_SILKIT_CONVERT(MetricsSink);
DEFINE_SILKIT_CONVERT(MetricsSink::Type);
DEFINE_SILKIT_CONVERT(Metrics);

DEFINE_SILKIT_CONVERT(Middleware);

DEFINE_SILKIT_CONVERT(Extensions);

DEFINE_SILKIT_CONVERT(Experimental);
DEFINE_SILKIT_CONVERT(TimeSynchronization);
DEFINE_SILKIT_CONVERT(Aggregation);

DEFINE_SILKIT_CONVERT(ParticipantConfiguration);

} // namespace YAML
