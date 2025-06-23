#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include "rapidyaml.hpp"

#include "ParticipantConfiguration.hpp"

namespace VSilKit {

//Utilities
bool IsValidChild(const ryml::ConstNodeRef& node, const std::string& name);

struct YamlReaderContext
{
    ryml::Parser& parser;
    ryml::ConstNodeRef node;
};

struct YamlReader
{
    YamlReaderContext _context;

public:
    YamlReader(YamlReaderContext cursor)
        : _context{std::move(cursor)}
    {
    }

private:
    // Helpers
    bool IsValid() const;
    bool IsMap() const;
    bool IsScalar() const;
    bool IsSequence() const;
    bool IsExistingString(const char* str) const;
    bool IsEmpty() const;
    bool IsString(const char* string) const;
    auto MakeConfigurationError(const char* message) const -> SilKit::ConfigurationError;
    auto GetChildSafe(const std::string& name) const -> YamlReader;
    auto MakeYamlReader(ryml::ConstNodeRef node) const -> YamlReader;

private:
    // Parsing utils
    template <typename T, typename std::enable_if_t<!std::is_integral<T>::value, bool> = true>
    void OptionalRead(T& val, const std::string& name)
    {
        auto&& child = GetChildSafe(name);
        if (child.IsValid())
        {
            child.Read(val);
        }
    }

    void OptionalRead(bool& val, const std::string& name);

    template <typename T>
    void Read(std::vector<T>& val)
    {
        for (auto&& i : _context.node.cchildren())
        {
            T element{};
            MakeYamlReader(i).Read(element);
            val.emplace_back(std::move(element));
        }
    }

    template <typename T>
    void OptionalRead(SilKit::Util::Optional<T>& val, const std::string& name)
    {
        auto&& child = GetChildSafe(name);
        if (child.IsValid())
        {
            T tmp;
            child.Read(tmp);
            val = std::move(tmp); // needs a proper setter to set "has_value"
        }
    }


    template <typename T, typename std::enable_if_t<std::is_integral<T>::value, bool> = true>
    void OptionalRead(T& val, const std::string& name)
    {
        auto tmp = ryml::fmt::overflow_checked(val);
        (void)_context.node.get_if(ryml::to_csubstr(name), &tmp);
    }


    template <typename T>
    void ReadController(T& obj)
    {
        ReadKeyValue(obj.name, "Name");
        OptionalRead(obj.network, "Network");
        OptionalRead(obj.useTraceSinks, "UseTraceSinks");
        OptionalRead(obj.replay, "Replay");
    }

    template <typename ConfigT>
    void OptionalRead_deprecated_alternative(ConfigT& value, const std::string& fieldName,
                                             std::initializer_list<std::string> deprecatedFieldNames)
    {
        if (!(IsMap() || IsSequence()))
        {
            return;
        }

        auto hasChild = [this](auto&& name) {
            auto&& c = GetChildSafe(name);
            return c.IsValid();
        };

        std::vector<std::string> presentDeprecatedFieldNames;
        std::copy_if(deprecatedFieldNames.begin(), deprecatedFieldNames.end(),
                     std::back_inserter(presentDeprecatedFieldNames), hasChild);

        if (IsValidChild(_context.node, fieldName) && presentDeprecatedFieldNames.size() >= 1)
        {
            std::stringstream ss;
            ss << "The key \"" << fieldName << "\" and the deprecated alternatives";
            for (const auto& deprecatedFieldName : presentDeprecatedFieldNames)
            {
                ss << " \"" << deprecatedFieldName << "\"";
            }
            ss << " are present.";
            throw SilKit::ConfigurationError{ss.str()};
        }

        if (presentDeprecatedFieldNames.size() >= 2)
        {
            std::stringstream ss;
            ss << "The deprecated keys";
            for (const auto& deprecatedFieldName : presentDeprecatedFieldNames)
            {
                ss << " \"" << deprecatedFieldName << "\"";
            }
            ss << " are present.";
            throw SilKit::ConfigurationError{ss.str()};
        }

        OptionalRead(value, fieldName);
        for (const auto& deprecatedFieldName : deprecatedFieldNames)
        {
            OptionalRead(value, deprecatedFieldName);
        }
    }
    template <typename T>
    void Read(T& value)
    {
        _context.node >> value;
    }

public:
    template <typename Rep, typename Period>
    void Read(std::chrono::duration<Rep, Period>& obj)
    {
        Rep value{};
        auto tmp = ryml::fmt::overflow_checked(value);
        Read(tmp);
        obj = std::chrono::milliseconds{value};
    }
    void Read(SilKit::Services::MatchingLabel& value);
    void Read(SilKit::Services::MatchingLabel::Kind& value);
    void Read(SilKit::Services::Logging::Level& obj);
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

public:
    //main parsing
    template <typename T>
    void ReadAll(T& value)
    {
        Read(value);
    }

    // XXX only needed in YamlValidator
    template <typename T>
    void ReadKeyValue(T& value, const std::string& name)
    {
        auto&& child = GetChildSafe(name);
        child.Read(value);
    }
};

} // namespace VSilKit
