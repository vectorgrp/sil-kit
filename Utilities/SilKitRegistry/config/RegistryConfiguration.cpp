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

#include "RegistryConfiguration.hpp"

// SIL Kit Registry Headers
#include "RegistryYamlConversion.hpp"

// Internal SIL Kit Headers
#include "YamlParser.hpp"

// Third-Party Headers
#include "fmt/format.h"
#include "yaml-cpp/yaml.h"


namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration
{
    YAML::Node node = YAML::Load(string);

    if (node.IsNull())
    {
        return V1::RegistryConfiguration{};
    }

    const auto schemaVersion = [&node]() -> std::string {
        const auto schemaVersionNode = node["SchemaVersion"];

        // if the 'SchemaVersion' is not set, assume a particular schema version.
        if (schemaVersionNode.Type() == YAML::NodeType::Undefined)
        {
            return V1::GetSchemaVersion();
        }

        if (!schemaVersionNode.IsScalar())
        {
            throw SilKit::ConfigurationError{
                "The 'SchemaVersion' field of the registry configuration must be a scalar value!"};
        }

        return schemaVersionNode.Scalar();
    }();

    if (schemaVersion == V1::GetSchemaVersion())
    {
        return SilKit::Config::from_yaml<V1::RegistryConfiguration>(node);
    }

    // After the 'active' schema version is bumped, parse and validate the then old configuration and transform it into
    // the 'active' configuration version.  The 'active' configuration version is the one returned by this function, as
    // stated in the namespace.

    throw SilKit::ConfigurationError{
        fmt::format("Unknown schema version '{}' found in registry configuration!", schemaVersion)};
}

} // namespace Config
} // namespace SilKitRegistry
