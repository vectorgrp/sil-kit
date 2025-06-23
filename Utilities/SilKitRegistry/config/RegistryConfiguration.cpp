/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "ParticipantConfiguration.hpp"
#include "RegistryConfiguration.hpp"

// Internal SIL Kit Headers
#include "YamlParser.hpp"
#include "fmt/format.h"

namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration
{
    auto&& configuration =  SilKit::Config::Deserialize<V1::RegistryConfiguration>(string);
    if (!configuration.schemaVersion.empty() && configuration.schemaVersion != SilKitRegistry::Config::V1::GetSchemaVersion())
    {
        throw SilKit::ConfigurationError{fmt::format("Unknown schema version '{}' found in registry configuration!", configuration.schemaVersion)};
    }
    return configuration;
}

} // namespace Config
} // namespace SilKitRegistry
