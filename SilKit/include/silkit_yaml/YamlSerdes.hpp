// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// UNSTABLE API: this auxiliary library does NOT carry the API/ABI stability
// guarantees of the main SIL Kit API (the silkit/ headers). Pin your SIL Kit
// version. See silkit_yaml/README for details.

#include <string>

#include "silkit/participant/exception.hpp"

#include "rapidyaml.hpp"

#include "silkit_yaml/YamlParserUtils.hpp"

namespace VSilKit {

//////////////////////////////////////////////////////////////////////
// Generic YAML/JSON (de)serialization
//
// R must derive from VSilKit::BasicYamlReader<R> and provide a
//   void Read(T&) overload for the target type T.
// W must derive from VSilKit::BasicYamlWriter<W> and provide a
//   void Write(const T&) overload for the source type T.
//////////////////////////////////////////////////////////////////////

template <typename T, typename R>
auto Deserialize(const std::string& input) -> T
{
    if (input.empty())
    {
        return {};
    }

    const auto rapidyamlCallbacks = VSilKit::GetRapidyamlCallbacks();

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
        throw SilKit::ConfigurationError{ex.what()};
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

} // namespace VSilKit
