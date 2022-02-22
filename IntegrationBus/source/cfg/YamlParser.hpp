// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "YamlConversion.hpp"

namespace ib {
namespace cfg {

//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//////////////////////////////////////////////////////////////////////

//!< Helper to print the YAML position in line and column format.
std::ostream& operator<<(std::ostream& out, const YAML::Mark& mark);

template<typename VibConfigT>
auto to_yaml(const VibConfigT& vibValue) -> YAML::Node
{
    YAML::Node node;
    try
    {
        node = vibValue;
        return node;
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark
            << ": " << ex.msg;
        throw configuration_error{ ss.str() };
    }
}
template<typename VibConfigT>
auto from_yaml(const YAML::Node& node) -> VibConfigT
{
    try
    {
        return node.as<VibConfigT>();
    }
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark
            << ": " << ex.msg;
        throw configuration_error{ ss.str() };
    }
}

//!< Convert a YAML document node into json, using the internal emitter.
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

} // namespace cfg
} // namespace ib
