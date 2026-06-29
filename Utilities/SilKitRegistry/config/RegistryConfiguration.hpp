// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

// Internal SIL Kit Headers
#include "config/Configuration.hpp"
#include "config/ParticipantConfiguration.hpp"


namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration;

} // namespace Config
} // namespace SilKitRegistry
