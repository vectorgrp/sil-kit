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
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark
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
    catch (const YAML::Exception& ex)
    {
        std::stringstream ss;
        ss << "YAML Error @ " << ex.mark
            << ": " << ex.msg;
        throw Misconfiguration{ ss.str() };
    }
}

} // namespace cfg
} // namespace ib
