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

#include "ParticipantConfiguration.hpp"

namespace VSilKit {

template <typename Impl>
struct BasicYamlWriter
{
    ryml::NodeRef node;

public:
    BasicYamlWriter(ryml::NodeRef node_)
        : node(node_)
    {
    }

public:
    template <typename T>
    void OptionalWrite(const std::optional<T>& val, const std::string& name)
    {
        if (val.has_value())
        {
            WriteKeyValue(name, val.value());
        }
    }

    template <typename T>
    void OptionalWrite(const std::vector<T>& val, const std::string& name)
    {
        if (!val.empty())
        {
            WriteKeyValue(name, val);
        }
    }

    void OptionalWrite(const std::string& val, const std::string& name)
    {
        if (!val.empty())
        {
            WriteKeyValue(name, val);
        }
    }

    template <typename T>
    void NonDefaultWrite(const T& val, const std::string& name, const T& defaultValue)
    {
        if (!(val == defaultValue))
        {
            WriteKeyValue(name, val);
        }
    }

    template <typename T>
    void WriteKeyValue(const std::string& name, const T& val)
    {
        if (!node.is_map())
        {
            throw SilKit::ConfigurationError("Parse error: trying to access child of something not a map");
        }

        auto writer = MakeImpl(node.append_child() << ryml::key(name));
        writer.Write(val);
    }

    template <typename T>
    void Write(const T& val)
    {
        node << val;
    }

    template <typename T>
    void Write(const std::vector<T>& val)
    {
        node |= ryml::SEQ;
        for (auto&& el : val)
        {
            auto writer = MakeImpl(node.append_child());
            writer.Write(el);
        }
    }

protected:
    void MakeMap()
    {
        node |= ryml::MAP;
    }

    auto MakeConfigurationError(const char* message) const -> SilKit::ConfigurationError
    {
        std::ostringstream s;

        s << "error writing configuration: " << message;

        return SilKit::ConfigurationError{s.str()};
    }

protected:
    auto MakeImpl(ryml::NodeRef node_) const -> Impl
    {
        return Impl{node_};
    }

private:
    auto AsImpl() -> Impl&
    {
        return static_cast<Impl&>(*this);
    }

    auto AsImpl() const -> const Impl&
    {
        return static_cast<const Impl&>(*this);
    }
};

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
