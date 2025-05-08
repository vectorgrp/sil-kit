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

// Internal SIL Kit Headers
#include "YamlParser.hpp"

// Third-Party Headers
#include "fmt/format.h"

namespace SilKitRegistry {
namespace Config {
namespace V1 {

bool operator==(const Experimental& lhs, const Experimental& rhs)
{
    return lhs.metrics == rhs.metrics;
}

} // namespace V1
} // namespace Config
} // namespace SilKitRegistry

namespace SilKitRegistry {
namespace Config {
namespace V1 {
using namespace SilKit::Config;
// rapid yaml impl

void write(ryml::NodeRef* node, const SilKitRegistry::Config::V1::Experimental& obj)
{
    static const V1::Experimental defaultObject{};
    NonDefaultWrite(obj.metrics, node, "Metrics", defaultObject.metrics);
}

bool read(const ryml::ConstNodeRef& node, SilKitRegistry::Config::V1::Experimental* obj)
{
    OptionalRead(obj->metrics, node, "Metrics");

    for (auto&& sink : obj->metrics.sinks)
    {
        if (sink.type == SilKit::Config::MetricsSink::Type::Remote)
        {
            throw SilKit::ConfigurationError{"SIL Kit Registry does not support remote metrics sinks"};
        }
    }

    return true;
}

void write(ryml::NodeRef* node, const SilKitRegistry::Config::V1::RegistryConfiguration& obj)
{
    static const V1::RegistryConfiguration defaultObject{};
    *node |= ryml::MAP;
    Write(node, "SchemaVersion", V1::GetSchemaVersion());
    NonDefaultWrite(obj.description, node, "Description", defaultObject.description);
    OptionalWrite(obj.listenUri, node, "ListenUri");
    OptionalWrite(obj.enableDomainSockets, node, "EnableDomainSockets");
    OptionalWrite(obj.dashboardUri, node, "DashboardUri");
    NonDefaultWrite(obj.logging, node, "Logging", defaultObject.logging);
    NonDefaultWrite(obj.experimental, node, "Experimental", defaultObject.experimental);
}

bool read(const ryml::ConstNodeRef& node, SilKitRegistry::Config::V1::RegistryConfiguration* obj)
{
    std::string schemaVersion;
    OptionalRead(schemaVersion, node, "SchemaVersion");
    if (!schemaVersion.empty() && schemaVersion != V1::GetSchemaVersion())
    {
        throw SilKit::ConfigurationError{
            fmt::format("Unknown schema version '{}' found in registry configuration!", schemaVersion)};
    }

    OptionalRead(obj->description, node, "Description");
    OptionalRead(obj->listenUri, node, "ListenUri");
    OptionalRead(obj->enableDomainSockets, node, "EnableDomainSockets");
    OptionalRead(obj->dashboardUri, node, "DashboardUri");
    OptionalRead(obj->logging, node, "Logging");
    OptionalRead(obj->experimental, node, "Experimental");

    if (obj->logging.logFromRemotes)
    {
        throw SilKit::ConfigurationError{"SIL Kit Registry does not support receiving logs from remotes"};
    }

    for (auto&& sink : obj->logging.sinks)
    {
        if (sink.type == SilKit::Config::Sink::Type::Remote)
        {
            throw SilKit::ConfigurationError{"SIL Kit Registry does not support remote logging"};
        }
    }
    return true;
}
} // namespace V1
} // namespace Config
} // namespace SilKitRegistry

namespace SilKitRegistry {
namespace Config {

auto Parse(const std::string& string) -> V1::RegistryConfiguration
{
    return SilKit::Config::DeserializeNew<V1::RegistryConfiguration>(string);
}

} // namespace Config
} // namespace SilKitRegistry
