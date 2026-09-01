#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Generic ryml tree-writing machinery, split out of YamlWriter.hpp so that consumers which only
// need the CRTP base (e.g. the dashboard JSON writer) do not have to pull in the whole of
// ParticipantConfiguration.hpp.

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "rapidyaml.hpp"

#include "silkit/participant/exception.hpp"

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

} // namespace VSilKit
