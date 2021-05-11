// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

namespace ib {
namespace cfg {

auto YamlToJson(const std::string& yamlString) -> std::string;
auto JsonToYaml(const std::string& jsonString) -> std::string;

} // namespace cfg
} // namespace ib
