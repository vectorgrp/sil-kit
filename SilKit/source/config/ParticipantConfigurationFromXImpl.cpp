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

#include <fstream>
#include <sstream>
#include <iomanip>

#include "YamlParser.hpp"
#include "YamlValidator.hpp"

namespace SilKit {
namespace Config {

inline namespace v1 {

// ================================================================================
//  Helper functions
// ================================================================================
namespace {

auto ReadFile(const std::string& filename) -> std::string
{
    std::ifstream fs(filename);

    if (!fs.is_open())
        throw SilKit::ConfigurationError("the file could not be opened");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return buffer.str();
}

auto Parse(const std::string& text) -> SilKit::Config::ParticipantConfiguration
{
    std::stringstream warnings;
    SilKit::Config::YamlValidator validator;
    if (!validator.Validate(text, warnings))
    {
        throw SilKit::ConfigurationError{"YAML validation returned errors: \n" + warnings.str()};
    }
    if (warnings.str().size() > 0)
    {
        std::cout << "YAML validation returned warnings: \n" << warnings.str() << std::endl;
    }
    YAML::Node doc = YAML::Load(text);

    auto configuration = !doc.IsNull() ? SilKit::Config::from_yaml<SilKit::Config::v1::ParticipantConfiguration>(doc)
                                       : SilKit::Config::v1::ParticipantConfiguration{};
    configuration.configurationFilePath.clear();

    return configuration;
}

} // anonymous namespace

// ================================================================================
//  Implementation data types
// ================================================================================
bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network;
}

bool operator==(const LinController& lhs, const LinController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const EthernetController& lhs, const EthernetController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const FlexrayController& lhs, const FlexrayController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.clusterParameters == rhs.clusterParameters
           && lhs.nodeParameters == rhs.nodeParameters && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const DataPublisher& lhs, const DataPublisher& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const DataSubscriber& lhs, const DataSubscriber& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const RpcServer& lhs, const RpcServer& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const RpcClient& lhs, const RpcClient& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const HealthCheck& lhs, const HealthCheck& rhs)
{
    return lhs.softResponseTimeout == rhs.softResponseTimeout && lhs.hardResponseTimeout == rhs.hardResponseTimeout;
}

bool operator==(const Tracing& lhs, const Tracing& rhs)
{
    return lhs.traceSinks == rhs.traceSinks && lhs.traceSources == rhs.traceSources;
}

bool operator==(const Extensions& lhs, const Extensions& rhs)
{
    return lhs.searchPathHints == rhs.searchPathHints;
}


bool operator==(const Middleware& lhs, const Middleware& rhs)
{
    return lhs.registryUri == rhs.registryUri && lhs.connectAttempts == rhs.connectAttempts
           && lhs.enableDomainSockets == rhs.enableDomainSockets && lhs.tcpNoDelay == rhs.tcpNoDelay
           && lhs.tcpQuickAck == rhs.tcpQuickAck && lhs.tcpReceiveBufferSize == rhs.tcpReceiveBufferSize
           && lhs.tcpSendBufferSize == rhs.tcpSendBufferSize && lhs.acceptorUris == rhs.acceptorUris;
}

bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs)
{
    return lhs.participantName == rhs.participantName && lhs.canControllers == rhs.canControllers
           && lhs.linControllers == rhs.linControllers && lhs.ethernetControllers == rhs.ethernetControllers
           && lhs.flexrayControllers == rhs.flexrayControllers && lhs.dataPublishers == rhs.dataPublishers
           && lhs.dataSubscribers == rhs.dataSubscribers && lhs.rpcClients == rhs.rpcClients
           && lhs.rpcServers == rhs.rpcServers && lhs.logging == rhs.logging && lhs.healthCheck == rhs.healthCheck
           && lhs.tracing == rhs.tracing && lhs.extensions == rhs.extensions;
}

} // inline namespace v1

auto ParticipantConfigurationFromStringImpl(const std::string& text)
-> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configuration = SilKit::Config::Parse(text);

    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

auto ParticipantConfigurationFromFileImpl(const std::string& filename)
-> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto text = SilKit::Config::ReadFile(filename);
    auto configuration = SilKit::Config::Parse(text);

    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

} // namespace Config
} // namespace SilKit
