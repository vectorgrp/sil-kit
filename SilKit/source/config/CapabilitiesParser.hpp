// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>
#include <vector>

namespace VSilKit {

// Utility for parsing key-value lists for protocol capabilities
auto ParseCapabilities(const std::string& input) -> std::vector<std::map<std::string, std::string>>;

} // namespace VSilKit
