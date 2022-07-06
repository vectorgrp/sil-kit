// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
DEFINE_SILKIT_CONVERT(SilKit::Core::Logging::Level);

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

DEFINE_SILKIT_CONVERT(Middleware);

DEFINE_SILKIT_CONVERT(Extensions);

DEFINE_SILKIT_CONVERT(ParticipantConfiguration);

} // namespace YAML
