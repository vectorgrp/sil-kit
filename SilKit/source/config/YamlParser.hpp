#pragma once
// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include "YamlReader.hpp"
#include "YamlWriter.hpp"
#include "rapidyaml.hpp"


namespace VSilKit {

// Utility for parsing key-value lists for protocol capabilities
auto ParseCapabilities(const std::string& input) -> std::vector<std::map<std::string, std::string>>;

} // namespace VSilKit


namespace SilKit {
namespace Config {

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//////////////////////////////////////////////////////////////////////

template <typename T>
auto Deserialize(const std::string& input) -> T
{
    if (input.empty())
    {
        return {};
    }

    ryml::ParserOptions options{};
    options.locations(true);

    ryml::EventHandlerTree eventHandler{};
    auto parser = ryml::Parser(&eventHandler, options);
    parser.reserve_locations(100u);
    auto&& cinput = ryml::to_csubstr(input);
    try
    {
        auto tree = ryml::parse_in_arena(&parser, cinput);
        auto root = tree.crootref();

        VSilKit::YamlReader reader{{parser, root}};
        T result;
        reader.ReadAll(result);
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


template <typename T>
auto Serialize(const T& input) -> std::string
{
    ryml::Tree t;
    VSilKit::YamlWriter writer{t.rootref()};
    writer.Write(input);
    return ryml::emitrs_yaml<std::string>(t);
}

template <typename T>
auto SerializeAsJson(const T& input) -> std::string
{
    ryml::Tree t;
    VSilKit::YamlWriter writer{t.rootref()};
    writer.Write(input);
    return ryml::emitrs_json<std::string>(t);
}

} // namespace Config
} // namespace SilKit
