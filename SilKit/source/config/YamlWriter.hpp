#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <optional>
#include <vector>

#include "rapidyaml.hpp"

#include "silkit_yaml/BasicYamlWriter.hpp"

#include "config/ParticipantConfiguration.hpp"

namespace VSilKit {

struct YamlWriter : BasicYamlWriter<YamlWriter>
{
    using BasicYamlWriter::BasicYamlWriter;
    using BasicYamlWriter::OptionalWrite;
    using BasicYamlWriter::Write;

public:
    void OptionalWrite(const SilKit::Config::Replay& value, const std::string& name)
    {
        if (value.useTraceSource.size() > 0)
        {
            WriteKeyValue(name, value);
        }
    }

public:
    void Write(const std::chrono::milliseconds& obj);
    void Write(const std::chrono::nanoseconds& obj);
    void Write(const SilKit::Services::MatchingLabel::Kind& obj);
    void Write(const SilKit::Services::MatchingLabel& obj);
    void Write(const SilKit::Services::Logging::Level& obj);
    void Write(const SilKit::Services::Flexray::FlexrayClusterParameters& obj);
    void Write(const SilKit::Services::Flexray::FlexrayNodeParameters& obj);
    void Write(const SilKit::Services::Flexray::FlexrayTxBufferConfig& obj);
    void Write(const SilKit::Services::Flexray::FlexrayChannel& obj);
    void Write(const SilKit::Services::Flexray::FlexrayClockPeriod& obj);
    void Write(const SilKit::Services::Flexray::FlexrayTransmissionMode& obj);
    void Write(const SilKit::Config::Sink::Type& obj);
    void Write(const SilKit::Config::Sink::Format& obj);
    void Write(const SilKit::Config::Sink& obj);
    void Write(const SilKit::Config::Logging& obj);
    void Write(const SilKit::Config::MetricsSink::Type& obj);
    void Write(const SilKit::Config::MetricsSink& obj);
    void Write(const SilKit::Config::Metrics& obj);
    void Write(const SilKit::Config::MdfChannel& obj);
    void Write(const SilKit::Config::Replay& obj);
    void Write(const SilKit::Config::Replay::Direction& obj);
    void Write(const SilKit::Config::CanController& obj);
    void Write(const SilKit::Config::LinController& obj);
    void Write(const SilKit::Config::EthernetController& obj);
    void Write(const SilKit::Config::FlexrayController& obj);
    void Write(const SilKit::Config::Label::Kind& obj);
    void Write(const SilKit::Config::Label& obj);
    void Write(const SilKit::Config::DataPublisher& obj);
    void Write(const SilKit::Config::DataSubscriber& obj);
    void Write(const SilKit::Config::RpcServer& obj);
    void Write(const SilKit::Config::RpcClient& obj);
    void Write(const SilKit::Config::Tracing& obj);
    void Write(const SilKit::Config::TraceSink& obj);
    void Write(const SilKit::Config::TraceSink::Type& obj);
    void Write(const SilKit::Config::TraceSource& obj);
    void Write(const SilKit::Config::TraceSource::Type& obj);
    void Write(const SilKit::Config::Extensions& obj);
    void Write(const SilKit::Config::Middleware& obj);
    void Write(const SilKit::Config::Includes& obj);
    void Write(const SilKit::Config::Aggregation& obj);
    void Write(const SilKit::Config::TimeSynchronization& obj);
    void Write(const SilKit::Config::Experimental& obj);
    void Write(const SilKit::Config::ParticipantConfiguration& obj);
    void Write(const SilKit::Config::HealthCheck& obj);
    void Write(const SilKitRegistry::Config::V1::Experimental& obj);
    void Write(const SilKitRegistry::Config::V1::RegistryConfiguration& obj);

    // used for debug logging
    void Write(const SilKit::Services::Flexray::FlexrayControllerConfig& obj);
};

} // namespace VSilKit
