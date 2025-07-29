// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantConfiguration.hpp"

#include <string>
#include <type_traits>

namespace SilKit {
namespace Config {
inline namespace V1 {

auto Label::ToPublicApi() const -> SilKit::Services::MatchingLabel
{
    SilKit::Services::MatchingLabel result;

    result.key = key;
    result.value = value;

    switch (kind)
    {
    case Kind::Mandatory:
        result.kind = SilKit::Services::MatchingLabel::Kind::Mandatory;
        break;
    case Kind::Optional:
        result.kind = SilKit::Services::MatchingLabel::Kind::Optional;
        break;
    default:
        throw SilKit::ConfigurationError{"Invalid SilKit::Config::v1::MatchingLabel::Kind("
                                         + std::to_string(static_cast<std::underlying_type_t<Label::Kind>>(kind))
                                         + ")"};
    }

    return result;
}


auto Label::FromPublicApi(const SilKit::Services::MatchingLabel& label) -> Label
{
    Label result;

    result.key = label.key;
    result.value = label.value;

    switch (label.kind)
    {
    case SilKit::Services::MatchingLabel::Kind::Mandatory:
        result.kind = Label::Kind::Mandatory;
        break;
    case SilKit::Services::MatchingLabel::Kind::Optional:
        result.kind = Label::Kind::Optional;
        break;
    default:
        throw SilKit::ConfigurationError{
            "Invalid SilKit::Services::MatchingLabel::Kind("
            + std::to_string(static_cast<std::underlying_type_t<SilKit::Services::MatchingLabel::Kind>>(label.kind))
            + ")"};
    }

    return result;
}


auto Label::VectorFromPublicApi(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::vector<Label>
{
    std::vector<SilKit::Config::V1::Label> result;
    std::transform(labels.begin(), labels.end(), std::back_inserter(result), Label::FromPublicApi);
    return result;
}

bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.replay == rhs.replay
           && lhs.useTraceSinks == rhs.useTraceSinks;
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

bool operator==(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return lhs.type == rhs.type && lhs.name == rhs.name;
}

bool operator==(const Metrics& lhs, const Metrics& rhs)
{
    return lhs.sinks == rhs.sinks && lhs.collectFromRemote == rhs.collectFromRemote;
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
           && lhs.tcpSendBufferSize == rhs.tcpSendBufferSize && lhs.acceptorUris == rhs.acceptorUris
           && lhs.registryAsFallbackProxy == rhs.registryAsFallbackProxy
           && lhs.connectTimeoutSeconds == rhs.connectTimeoutSeconds
           && lhs.experimentalRemoteParticipantConnection == rhs.experimentalRemoteParticipantConnection;
}

bool operator==(const Includes& lhs, const Includes& rhs)
{
    return lhs.files == rhs.files && lhs.searchPathHints == rhs.searchPathHints;
}

bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs)
{
    return lhs.participantName == rhs.participantName && lhs.canControllers == rhs.canControllers
           && lhs.linControllers == rhs.linControllers && lhs.ethernetControllers == rhs.ethernetControllers
           && lhs.flexrayControllers == rhs.flexrayControllers && lhs.dataPublishers == rhs.dataPublishers
           && lhs.dataSubscribers == rhs.dataSubscribers && lhs.rpcClients == rhs.rpcClients
           && lhs.rpcServers == rhs.rpcServers && lhs.logging == rhs.logging && lhs.healthCheck == rhs.healthCheck
           && lhs.tracing == rhs.tracing && lhs.extensions == rhs.extensions && lhs.experimental == rhs.experimental
           && lhs.includes == rhs.includes;
}

bool operator==(const TimeSynchronization& lhs, const TimeSynchronization& rhs)
{
    return lhs.animationFactor == rhs.animationFactor && lhs.enableMessageAggregation == rhs.enableMessageAggregation;
}

bool operator==(const Experimental& lhs, const Experimental& rhs)
{
    return lhs.timeSynchronization == rhs.timeSynchronization && lhs.metrics == rhs.metrics;
}

bool operator==(const Label& lhs, const Label& rhs)
{
    return lhs.key == rhs.key && lhs.value == rhs.value && lhs.kind == rhs.kind;
}

bool operator==(const SimulatedNetwork& lhs, const SimulatedNetwork& rhs)
{
    return lhs.name == rhs.name && lhs.type == rhs.type && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

auto operator<<(std::ostream& out, const Label::Kind& kind) -> std::ostream&
{
    switch (kind)
    {
    case Label::Kind::Mandatory:
        return out << "Mandatory";
    case Label::Kind::Optional:
        return out << "Optional";
    default:
        return out << "MatchingLabel::Kind(" << static_cast<std::underlying_type_t<Label::Kind>>(kind) << ")";
    }
}

auto operator<<(std::ostream& out, const Label& label) -> std::ostream&
{
    return out << "MatchingLabel{" << label.key << ", " << label.value << ", " << label.kind << "}";
}

bool operator<(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return std::make_tuple(lhs.type, lhs.name) < std::make_tuple(rhs.type, rhs.name);
}

bool operator>(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return !(lhs < rhs);
}

} // namespace V1
} // namespace Config
} // namespace SilKit

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
