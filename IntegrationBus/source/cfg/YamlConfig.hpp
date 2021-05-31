// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include <vector>

namespace ib {
namespace cfg {

//!< Converts the YAML string into a valid JSON string.
// This performs a validation step internally and prints warnings on std::cerr if any issues are found.
auto YamlToJson(const std::string& yamlString) -> std::string;
//!< Converts the JSON string into a YAML representation.
auto JsonToYaml(const std::string& jsonString) -> std::string;

//!< Check that the YAML document `yamlString` is a valid VIB config or emit warnings if issues were found.
bool Validate(const std::string& yamlString, std::ostream& warningMessages);

} // namespace cfg
} // namespace ib
