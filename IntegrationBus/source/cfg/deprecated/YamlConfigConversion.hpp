// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <chrono>
#include <string>
#include <sstream>

#include "yaml-cpp/yaml.h"

#include "VibYamlHelper.hpp"

// YAML-cpp serialization/deserialization for our config data types
namespace YAML {
using namespace ib::cfg;


DEFINE_VIB_CONVERT(MdfChannel);
DEFINE_VIB_CONVERT(Version);
DEFINE_VIB_CONVERT(Sink::Type);
DEFINE_VIB_CONVERT(ib::mw::logging::Level);
DEFINE_VIB_CONVERT(Sink);
DEFINE_VIB_CONVERT(Logger);
DEFINE_VIB_CONVERT(CanController);
DEFINE_VIB_CONVERT(LinController);
DEFINE_VIB_CONVERT(EthernetController);
DEFINE_VIB_CONVERT(ib::sim::fr::ClusterParameters);
DEFINE_VIB_CONVERT(ib::sim::fr::NodeParameters);
DEFINE_VIB_CONVERT(ib::sim::fr::TxBufferConfig);
DEFINE_VIB_CONVERT(ib::sim::fr::Channel);
DEFINE_VIB_CONVERT(ib::sim::fr::ClockPeriod);
DEFINE_VIB_CONVERT(ib::sim::fr::TransmissionMode);
DEFINE_VIB_CONVERT(FlexrayController);
DEFINE_VIB_CONVERT(GenericPort);
DEFINE_VIB_CONVERT(GenericPort::ProtocolType);
DEFINE_VIB_CONVERT(DataPort);
DEFINE_VIB_CONVERT(RpcPort);
DEFINE_VIB_CONVERT(SyncType);
DEFINE_VIB_CONVERT(std::chrono::milliseconds);
DEFINE_VIB_CONVERT(std::chrono::nanoseconds);
DEFINE_VIB_CONVERT(ParticipantController);
DEFINE_VIB_CONVERT(Participant);
DEFINE_VIB_CONVERT(Switch::Port);
DEFINE_VIB_CONVERT(Switch);
DEFINE_VIB_CONVERT(Link);
DEFINE_VIB_CONVERT(NetworkSimulator);
DEFINE_VIB_CONVERT(TimeSync::SyncPolicy);
DEFINE_VIB_CONVERT(TimeSync);
DEFINE_VIB_CONVERT(SimulationSetup);
DEFINE_VIB_CONVERT(FastRtps::DiscoveryType);
DEFINE_VIB_CONVERT(FastRtps::Config);
DEFINE_VIB_CONVERT(VAsio::RegistryConfig);
DEFINE_VIB_CONVERT(VAsio::Config);
DEFINE_VIB_CONVERT(Middleware);
DEFINE_VIB_CONVERT(MiddlewareConfig);
DEFINE_VIB_CONVERT(ExtensionConfig);
DEFINE_VIB_CONVERT(Config);
DEFINE_VIB_CONVERT(TraceSink);
DEFINE_VIB_CONVERT(TraceSink::Type);
DEFINE_VIB_CONVERT(TraceSource);
DEFINE_VIB_CONVERT(TraceSource::Type);
DEFINE_VIB_CONVERT(Replay);
DEFINE_VIB_CONVERT(Replay::Direction);

} //end YAML
