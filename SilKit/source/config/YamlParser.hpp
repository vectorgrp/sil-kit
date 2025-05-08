/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "YamlConversion.hpp"

namespace SilKit {
namespace Config {

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//////////////////////////////////////////////////////////////////////

//! Helper to print the YAML position in line and column format.
std::ostream& operator<<(std::ostream& out, const YAML::Mark& mark);

template <typename SilKitConfigT>
auto to_yaml(const SilKitConfigT& silkitValue) -> YAML::Node
{
    YAML::Node node;
    try
    {
        node = silkitValue;
        return node;
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark << ": " << ex.msg;
        throw ConfigurationError{ss.str()};
    }
}
template <typename SilKitConfigT>
auto from_yaml(const YAML::Node& node) -> SilKitConfigT
{
    try
    {
        return node.as<SilKitConfigT>();
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark << ": " << ex.msg;
        throw ConfigurationError{ss.str()};
    }
}

//! Convert a YAML document node into json, using the internal emitter.
auto yaml_to_json(YAML::Node node) -> std::string;


template <typename T>
auto Serialize(const T& value) -> std::string
{
    return YAML::Dump(to_yaml<T>(value));
}

template <typename T>
auto Deserialize(const std::string& str) -> T
{
    std::stringstream ss;
    ss << str;
    return from_yaml<T>(YAML::Load(ss));
}


template<typename T>
auto DeserializeNew(const std::string& input) -> T
{
    if (input.empty())
    {
        return {};
    }

    ryml::Callbacks cb{};

    cb.m_error = [](auto* msg, auto msg_len, auto location, auto* userdata) 
    {
        std::string message{ msg, msg_len };
        std::string errorMessage = Format("YAML Parsing error in file '{}' at line {} (offset {}), column {}: {}", location.name, location.line, location.offset, location.col, message);
        if (userdata)
        {
            auto&& ctx = reinterpret_cast<ParserContext*>(userdata);
            errorMessage = Format("YAML Parsing error in at line {} (offset {}), column {} near '{}' with error: '{}'. Expected value: '{}'",
                ctx->currentLocation.line, ctx->currentLocation.offset, ctx->currentLocation.col, ctx->currentContent, message, ctx->expectedValue);
        }
        throw SilKitError{ errorMessage };
    };

    ryml::ParserOptions options{};
    options.locations(true);

    ryml::EventHandlerTree eventHandler{cb};
    auto parser = ryml::Parser(&eventHandler, options);
    parser.reserve_locations(100u);
    auto&& cinput = ryml::to_csubstr(input);

    auto t = ryml::parse_in_arena(&parser, cinput);

    ParserContext ctx;
    ctx.parser = &parser;
    cb.m_user_data = &ctx;
    t.callbacks(cb);

    T result;
    t.crootref() >> result;

    return result;
}


template<typename T>
auto SerializeNew(const T& input) -> std::string
{
    ryml::Tree t;
    t.rootref() << input;
    return ryml::emitrs_yaml<std::string>(t);
}


} // namespace Config
} // namespace SilKit
