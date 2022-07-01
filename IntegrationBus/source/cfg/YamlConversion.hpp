// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>

#include "ParticipantConfiguration.hpp"

#include "yaml-cpp/yaml.h"

#include "VibYamlHelper.hpp"

// YAML-cpp serialization/deserialization for ParticipantConfiguration data types
namespace YAML {

using namespace ib::cfg;

DEFINE_VIB_CONVERT(std::chrono::milliseconds);
DEFINE_VIB_CONVERT(std::chrono::nanoseconds);

DEFINE_VIB_CONVERT(Logging);
DEFINE_VIB_CONVERT(Sink);
DEFINE_VIB_CONVERT(Sink::Type);
DEFINE_VIB_CONVERT(ib::mw::logging::Level);

DEFINE_VIB_CONVERT(MdfChannel);
DEFINE_VIB_CONVERT(Replay);
DEFINE_VIB_CONVERT(Replay::Direction);

DEFINE_VIB_CONVERT(CanController);

DEFINE_VIB_CONVERT(LinController);

DEFINE_VIB_CONVERT(EthernetController);

DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayClusterParameters);
DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayNodeParameters);
DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayTxBufferConfig);
DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayChannel);
DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayClockPeriod);
DEFINE_VIB_CONVERT(ib::sim::fr::FlexrayTransmissionMode);
DEFINE_VIB_CONVERT(FlexrayController);

DEFINE_VIB_CONVERT(DataPublisher);
DEFINE_VIB_CONVERT(DataSubscriber);
DEFINE_VIB_CONVERT(RpcServer);
DEFINE_VIB_CONVERT(RpcClient);

DEFINE_VIB_CONVERT(HealthCheck);

DEFINE_VIB_CONVERT(Tracing);
DEFINE_VIB_CONVERT(TraceSink);
DEFINE_VIB_CONVERT(TraceSink::Type);
DEFINE_VIB_CONVERT(TraceSource);
DEFINE_VIB_CONVERT(TraceSource::Type);

DEFINE_VIB_CONVERT(Middleware);

DEFINE_VIB_CONVERT(Extensions);

DEFINE_VIB_CONVERT(ParticipantConfiguration);

} // namespace YAML
