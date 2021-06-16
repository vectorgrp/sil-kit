// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "YamlConfigConversion.hpp"



namespace ib {
namespace cfg {

//!< Converts the YAML string into a valid JSON string.
// This performs a validation step internally and prints warnings on std::cerr if any issues are found.
auto YamlToJson(const std::string& yamlString) -> std::string;
//!< Converts the JSON string into a YAML representation.
auto JsonToYaml(const std::string& jsonString) -> std::string;

//!< Check that the YAML document `yamlString` is a valid VIB config or emit warnings if issues were found.
bool Validate(const std::string& yamlString, std::ostream& warningMessages);


//////////////////////////////////////////////////////////////////////
// Configuration Parsing
//////////////////////////////////////////////////////////////////////


//!< Helper to rint the YAML position in line and column format.
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
    catch (const YAML::BadConversion& ex)
    {
        std::stringstream ss;
        ss << "YAML Misconfiguration @ " << ex.mark
            << ": " << ex.msg;
        throw Misconfiguration{ ss.str() };
    }
}
template<typename VibConfigT>
auto from_yaml(const YAML::Node& node) -> VibConfigT
{
    try
    {
        return node.as<VibConfigT>();
    }
    catch (const YAML::BadConversion& ex)
    {
        std::stringstream ss;
        ss << "YAML Misconfiguration @ " << ex.mark
            << ": " << ex.msg;
        throw Misconfiguration{ ss.str() };
    }
}

} // namespace cfg
} // namespace ib
