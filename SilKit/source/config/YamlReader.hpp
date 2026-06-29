#pragma once

// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <memory>
#include <optional>
#include <sstream>
#include <map>
#include <vector>

#include "rapidyaml.hpp"

#include "silkit_yaml/BasicYamlReader.hpp"

#include "config/ParticipantConfiguration.hpp"

namespace VSilKit {

struct YamlReader : BasicYamlReader<YamlReader>
{
    using BasicYamlReader::BasicYamlReader;
    using BasicYamlReader::Read;

protected: // Parsing utils
    template <typename T>
    void ReadController(T& obj)
    {
        ReadKeyValue(obj.name, "Name");
        OptionalRead(obj.network, "Network");
        OptionalRead(obj.useTraceSinks, "UseTraceSinks");
        OptionalRead(obj.replay, "Replay");
    }

public:
    void Read(SilKit::Services::MatchingLabel& value);
    void Read(SilKit::Services::MatchingLabel::Kind& value);
    void Read(SilKit::Services::Logging::Level& obj);
    void Read(SilKit::Services::Logging::Topic& obj);
    void Read(SilKit::Services::Flexray::FlexrayClusterParameters& obj);
    void Read(SilKit::Services::Flexray::FlexrayNodeParameters& obj);
    void Read(SilKit::Services::Flexray::FlexrayTxBufferConfig& obj);
    void Read(SilKit::Services::Flexray::FlexrayChannel& obj);
    void Read(SilKit::Services::Flexray::FlexrayClockPeriod& obj);
    void Read(SilKit::Services::Flexray::FlexrayTransmissionMode& obj);
    void Read(SilKit::Config::Sink::Type& obj);
    void Read(SilKit::Config::Sink::Format& obj);
    void Read(SilKit::Config::Sink& obj);
    void Read(SilKit::Config::Logging& obj);
    void Read(SilKit::Config::MetricsSink::Type& obj);
    void Read(SilKit::Config::MetricsSink& obj);
    void Read(SilKit::Config::Metrics& obj);
    void Read(SilKit::Config::MdfChannel& obj);
    void Read(SilKit::Config::Replay& obj);
    void Read(SilKit::Config::Replay::Direction& obj);
    void Read(SilKit::Config::CanController& obj);
    void Read(SilKit::Config::LinController& obj);
    void Read(SilKit::Config::EthernetController& obj);
    void Read(SilKit::Config::FlexrayController& obj);
    void Read(SilKit::Config::Label::Kind& obj);
    void Read(SilKit::Config::Label& obj);
    void Read(SilKit::Config::DataPublisher& obj);
    void Read(SilKit::Config::DataSubscriber& obj);
    void Read(SilKit::Config::RpcServer& obj);
    void Read(SilKit::Config::RpcClient& obj);
    void Read(SilKit::Config::Tracing& obj);
    void Read(SilKit::Config::TraceSink& obj);
    void Read(SilKit::Config::TraceSink::Type& obj);
    void Read(SilKit::Config::TraceSource& obj);
    void Read(SilKit::Config::TraceSource::Type& obj);
    void Read(SilKit::Config::Extensions& obj);
    void Read(SilKit::Config::Middleware& obj);
    void Read(SilKit::Config::Includes& obj);
    void Read(SilKit::Config::Aggregation& obj);
    void Read(SilKit::Config::TimeSynchronization& obj);
    void Read(SilKit::Config::Experimental& obj);
    void Read(SilKit::Config::ParticipantConfiguration& obj);
    void Read(SilKit::Config::HealthCheck& obj);

    //Registry
    void Read(SilKitRegistry::Config::V1::Experimental& obj);
    void Read(SilKitRegistry::Config::V1::RegistryConfiguration& obj);
};

} // namespace VSilKit
