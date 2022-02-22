// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <chrono>
#include <string>

#include "ParticipantConfiguration.hpp"

#include "yaml-cpp/yaml.h"

// YAML-cpp serialization/deserialization for ParticipantConfiguration data types
namespace YAML {

using namespace ib::cfg::v1::datatypes;

// Encode/Decode implementation is provided as templated static methods, to reduce boiler plate code.
struct Converter
{
    // required for YAML::convert<T>:
    template<typename IbDataType>
    static Node encode(const IbDataType& obj);
    template<typename IbDataType>
    static bool decode(const Node& node, IbDataType& obj);
};

template<> struct convert<std::chrono::milliseconds> : public Converter {};
template<> struct convert<std::chrono::nanoseconds> : public Converter {};

template<> struct convert<Logging> : public Converter {};
template<> struct convert<Sink> : public Converter {};
template<> struct convert<Sink::Type> : public Converter {};
template<> struct convert<ib::mw::logging::Level> : public Converter {};

template<> struct convert<MdfChannel> : public Converter {};
template<> struct convert<Replay> : public Converter {};
template<> struct convert<Replay::Direction> : public Converter {};

template<> struct convert<CanController> : public Converter {};

template<> struct convert<LinController> : public Converter {};

template<> struct convert<EthernetController> : public Converter {};

template<> struct convert<ib::sim::fr::ClusterParameters> : public Converter {};
template<> struct convert<ib::sim::fr::NodeParameters> : public Converter {};
template<> struct convert<ib::sim::fr::TxBufferConfig> : public Converter {};
template<> struct convert<ib::sim::fr::Channel> : public Converter {};
template<> struct convert<ib::sim::fr::ClockPeriod> : public Converter {};
template<> struct convert<ib::sim::fr::TransmissionMode> : public Converter {};
template<> struct convert<FlexRayController> : public Converter {};

template<> struct convert<DataPublisher> : public Converter {};
template<> struct convert<DataSubscriber> : public Converter {};
template<> struct convert<RpcServer> : public Converter {};
template<> struct convert<RpcClient> : public Converter {};

template<> struct convert<HealthCheck> : public Converter {};

template<> struct convert<Tracing> : public Converter {};
template<> struct convert<TraceSink> : public Converter {};
template<> struct convert<TraceSink::Type> : public Converter {};
template<> struct convert<TraceSource> : public Converter {};
template<> struct convert<TraceSource::Type> : public Converter {};

template<> struct convert<Registry> : public Converter {};
template<> struct convert<Middleware> : public Converter {};

template<> struct convert<Extensions> : public Converter {};

template<> struct convert<ParticipantConfiguration> : public Converter {};

} // namespace YAML
