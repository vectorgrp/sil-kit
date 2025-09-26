// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantConfiguration.hpp"
#include "RegistryConfiguration.hpp"

// Internal SIL Kit Headers
#include "YamlParser.hpp"
#include "fmt/format.h"

namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration
{
    auto&& configuration = SilKit::Config::Deserialize<V1::RegistryConfiguration>(string);
    if (!configuration.schemaVersion.empty()
        && configuration.schemaVersion != SilKitRegistry::Config::V1::GetSchemaVersion())
    {
        throw SilKit::ConfigurationError{
            fmt::format("Unknown schema version '{}' found in registry configuration!", configuration.schemaVersion)};
    }
    return configuration;
}

} // namespace Config
} // namespace SilKitRegistry
