// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include <chrono>

#include "yaml-cpp/yaml.h"

#include "Config.hpp"

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//////////////////////////////////////////////////////////////////////

// YAML serialization/deserialization for our data types
namespace YAML {
    using namespace ib::cfg;

    // Encode/Decode implementation is provided as templated static methods, to reduce boiler plate code.
    struct VibConversion
    {
        template<typename IbDataType>
        static Node encode(const IbDataType & obj);
        template<typename IbDataType>
        static bool decode(const Node& node, IbDataType& obj);
    };

    template<> struct convert<MdfChannel> : public VibConversion {};
    template<> struct convert<Version> : public VibConversion {};
    template<> struct convert<Sink::Type> : public VibConversion {};
    template<> struct convert<ib::mw::logging::Level> : public VibConversion {};
    template<> struct convert<Sink> : public VibConversion {};
    template<> struct convert<Logger> : public VibConversion {};
    template<> struct convert<CanController> : public VibConversion {};
    template<> struct convert<LinController> : public VibConversion {};
    template<> struct convert<EthernetController> : public VibConversion {};
    template<> struct convert<ib::sim::fr::ClusterParameters> : public VibConversion {};
    template<> struct convert<ib::sim::fr::NodeParameters> : public VibConversion {};
    template<> struct convert<ib::sim::fr::TxBufferConfig> : public VibConversion {};
    template<> struct convert<ib::sim::fr::Channel> : public VibConversion {};
    template<> struct convert<ib::sim::fr::ClockPeriod> : public VibConversion {};
    template<> struct convert<ib::sim::fr::TransmissionMode> : public VibConversion {};
    template<> struct convert<FlexrayController> : public VibConversion {};
    template<> struct convert<DigitalIoPort> : public VibConversion {};
    template<> struct convert<AnalogIoPort> : public VibConversion {};
    template<> struct convert<PwmPort> : public VibConversion {};
    template<> struct convert<PatternPort> : public VibConversion {};
    template<> struct convert<GenericPort> : public VibConversion {};
    template<> struct convert<GenericPort::ProtocolType> : public VibConversion {};
    template<> struct convert<SyncType> : public VibConversion {};
    template<> struct convert<std::chrono::milliseconds> : public VibConversion {};
    template<> struct convert<std::chrono::nanoseconds> : public VibConversion {};
    template<> struct convert<ParticipantController> : public VibConversion {};
    template<> struct convert<Participant> : public VibConversion {};
    template<> struct convert<Switch::Port> : public VibConversion {};
    template<> struct convert<Switch> : public VibConversion {};
    template<> struct convert<Link> : public VibConversion {};
    template<> struct convert<NetworkSimulator> : public VibConversion {};
    template<> struct convert<TimeSync::SyncPolicy> : public VibConversion {};
    template<> struct convert<TimeSync> : public VibConversion {};
    template<> struct convert<SimulationSetup> : public VibConversion {};
    template<> struct convert<FastRtps::DiscoveryType> : public VibConversion {};
    template<> struct convert<FastRtps::Config> : public VibConversion {};
    template<> struct convert<VAsio::RegistryConfig> : public VibConversion {};
    template<> struct convert<VAsio::Config> : public VibConversion {};
    template<> struct convert<Middleware> : public VibConversion {};
    template<> struct convert<MiddlewareConfig> : public VibConversion {};
    template<> struct convert<ExtensionConfig> : public VibConversion {};
    template<> struct convert<Config> : public VibConversion {};
    template<> struct convert<TraceSink> : public VibConversion {};
    template<> struct convert<TraceSink::Type> : public VibConversion {};
    template<> struct convert<TraceSource> : public VibConversion {};
    template<> struct convert<TraceSource::Type> : public VibConversion {};
    template<> struct convert<Replay> : public VibConversion {};
    template<> struct convert<Replay::Direction> : public VibConversion {};
} //end YAML

namespace ib {
namespace cfg {

//!< Converts the YAML string into a valid JSON string.
// This performs a validation step internally and prints warnings on std::cerr if any issues are found.
auto YamlToJson(const std::string& yamlString) -> std::string;
//!< Converts the JSON string into a YAML representation.
auto JsonToYaml(const std::string& jsonString) -> std::string;

//!< Check that the YAML document `yamlString` is a valid VIB config or emit warnings if issues were found.
bool Validate(const std::string& yamlString, std::ostream& warningMessages);


} // namespace cfg
} // namespace ib
