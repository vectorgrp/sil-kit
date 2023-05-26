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

#include "RegistryYamlConversion.hpp"
#include "YamlConversion.hpp"


namespace {

using namespace SilKitRegistry::Config;

using SilKit::Config::optional_encode;
using SilKit::Config::optional_decode;

using SilKit::Config::non_default_encode;

} // namespace


namespace YAML {

template <>
Node Converter::encode(const V1::RegistryConfiguration& obj)
{
    static const V1::RegistryConfiguration defaultObj{};

    Node node;
    node["SchemaVersion"] = V1::GetSchemaVersion();

    non_default_encode(obj.description, node, "Description", defaultObj.description);
    optional_encode(obj.listenUri, node, "ListenUri");
    optional_encode(obj.enableDomainSockets, node, "EnableDomainSockets");
    optional_encode(obj.dashboardUri, node, "DashboardUri");
    non_default_encode(obj.logging, node, "Logging", defaultObj.logging);

    return node;
}

template <>
bool Converter::decode(const Node& node, V1::RegistryConfiguration& obj)
{
    optional_decode(obj.description, node, "Description");
    optional_decode(obj.listenUri, node, "ListenUri");
    optional_decode(obj.enableDomainSockets, node, "EnableDomainSockets");
    optional_decode(obj.dashboardUri, node, "DashboardUri");
    optional_decode(obj.logging, node, "Logging");

    if (obj.logging.logFromRemotes)
    {
        throw SilKit::ConfigurationError{"SIL Kit Registry does not support receiving logs from remotes"};
    }

    for (const auto& sink : obj.logging.sinks)
    {
        if (sink.type == SilKit::Config::Sink::Type::Remote)
        {
            throw SilKit::ConfigurationError{"SIL Kit Registry does not support remote logging"};
        }
    }

    return true;
}

} // namespace YAML
