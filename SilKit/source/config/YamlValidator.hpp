// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <ostream>


namespace SilKit {
namespace Config {

//! YamlValidator is able to check the element <-> sub-element relations in a YAML document
bool ValidateWithSchema(const std::string& yamlString, std::ostream& warnings);

} // namespace Config
} // namespace SilKit
