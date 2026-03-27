// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>
#include <vector>

#include "silkit/participant/exception.hpp"

#include "rapidyaml.hpp"

namespace VSilKit {

// Utility for parsing key-value lists for protocol capabilities
auto ParseCapabilities(const std::string& input) -> std::vector<std::map<std::string, std::string>>;

auto MakeConfigurationError(ryml::Location location, const std::string_view message) -> SilKit::ConfigurationError;

auto GetRapidyamlCallbacks() -> ryml::Callbacks;

} // namespace VSilKit
