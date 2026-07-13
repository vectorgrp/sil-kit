// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// SilKitYaml: self-contained, header-only YAML/JSON (de)serialization. See README.md.

#include <string>

#include "rapidyaml.hpp"

#include "SilKitYaml/YamlError.hpp"
#include "SilKitYaml/YamlParserUtils.hpp"

namespace SilKitYaml {

//////////////////////////////////////////////////////////////////////
// Generic YAML/JSON (de)serialization
//
// R must derive from SilKitYaml::BasicYamlReader<R> and provide a
//   void Read(T&) overload for the target type T.
// W must derive from SilKitYaml::BasicYamlWriter<W> and provide a
//   void Write(const T&) overload for the source type T.
//
// All errors are reported as SilKitYaml::YamlError.
//////////////////////////////////////////////////////////////////////

template <typename T, typename R>
auto Deserialize(const std::string& input) -> T
{
    if (input.empty())
    {
        return {};
    }

    const auto rapidyamlCallbacks = SilKitYaml::GetRapidyamlCallbacks();

    ryml::ParserOptions options{};
    options.locations(true);

    ryml::EventHandlerTree eventHandler{};
    auto parser = ryml::Parser(&eventHandler, options);
    parser.reserve_locations(100u);
    auto&& cinput = ryml::to_csubstr(input);
    try
    {
        auto tree = ryml::parse_in_arena(&parser, cinput);

        // Install the error-handling callbacks. This will nicely format errors and throw an exception.
        tree.callbacks(rapidyamlCallbacks);

        // Extract a reference to the root node of the document tree.
        auto root = tree.crootref();

        R reader{parser, root};
        T result{};
        reader.Read(result);
        return result;
    }
    catch (const std::exception& ex)
    {
        throw YamlError{ex.what()};
    }
    catch (...)
    {
        throw;
    }
}

template <typename T, typename W>
auto Serialize(const T& input) -> std::string
{
    ryml::Tree t;
    W writer{t.rootref()};
    writer.Write(input);
    return ryml::emitrs_yaml<std::string>(t);
}

template <typename T, typename W>
auto SerializeAsJson(const T& input) -> std::string
{
    ryml::Tree t;
    W writer{t.rootref()};
    writer.Write(input);
    return ryml::emitrs_json<std::string>(t);
}

} // namespace SilKitYaml
