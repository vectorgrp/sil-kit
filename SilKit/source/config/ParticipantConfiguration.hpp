// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"
#include "silkit/services/datatypes.hpp"

#include "Configuration.hpp"

namespace SilKit {
namespace Config {
inline namespace V1 {

// ================================================================================
//  Internal controller service
// ================================================================================

//! \brief Generic dummy for all internal controllers - do not make available to public API!
struct InternalController
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::Undefined;
    }

    std::string name;
    std::optional<std::string> network;
};

// ================================================================================
//  CAN controller service
// ================================================================================

//! \brief CAN controller service
struct CanController
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::CAN;
    }

    std::string name;
    std::optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  LIN controller service
// ================================================================================

//! \brief LIN controller service
struct LinController
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::LIN;
    }

    std::string name;
    std::optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Ethernet controller service
// ================================================================================

//! \brief Ethernet controller service
struct EthernetController
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::Ethernet;
    }

    std::string name;
    std::optional<std::string> network;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  FlexRay controller service
// ================================================================================

//! \brief FlexRay controller service
struct FlexrayController
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::FlexRay;
    }

    std::string name;
    std::optional<std::string> network;

    std::optional<Services::Flexray::FlexrayClusterParameters> clusterParameters;
    std::optional<Services::Flexray::FlexrayNodeParameters> nodeParameters;
    std::vector<Services::Flexray::FlexrayTxBufferConfig> txBufferConfigurations;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Labels for Data Publisher/Subscriber and Rpc Servcer/Client service
// ================================================================================

struct Label
{
    enum struct Kind
    {
        Optional,
        Mandatory,
    };

    std::string key;
    std::string value;
    Kind kind;

    auto ToPublicApi() const -> SilKit::Services::MatchingLabel;
    static auto FromPublicApi(const SilKit::Services::MatchingLabel& label) -> Label;
    static auto VectorFromPublicApi(const std::vector<SilKit::Services::MatchingLabel>& labels) -> std::vector<Label>;
};

// ================================================================================
//  Data Publisher/Subscriber service
// ================================================================================

//! \brief Publisher configuration for the Data communication service
struct DataPublisher
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::Data;
    }

    std::string name;
    std::optional<std::string> topic;
    std::optional<std::vector<Label>> labels;

    //! \brief History length of a DataPublisher.
    std::optional<size_t> history;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

//! \brief Subscriber configuration for the Data communication service
struct DataSubscriber
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::Data;
    }

    std::string name;
    std::optional<std::string> topic;
    std::optional<std::vector<Label>> labels;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  RPC Client/Server service
// ================================================================================

//! \brief Server configuration for the RPC communication service
struct RpcServer
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::RPC;
    }

    std::string name;
    std::optional<std::string> functionName;
    std::optional<std::vector<Label>> labels;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

//! \brief Client configuration for the RPC communication service
struct RpcClient
{
    static constexpr auto GetNetworkType() -> NetworkType
    {
        return NetworkType::RPC;
    }

    std::string name;
    std::optional<std::string> functionName;
    std::optional<std::vector<Label>> labels;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Health checking service
// ================================================================================

//! \brief Health checking service
struct HealthCheck
{
    std::optional<std::chrono::milliseconds> softResponseTimeout;
    std::optional<std::chrono::milliseconds> hardResponseTimeout;
};

// ================================================================================
//  Tracing service
// ================================================================================

//! \brief Structure that contains a participant's setup of the tracing service
struct Tracing
{
    std::vector<TraceSink> traceSinks;
    std::vector<TraceSource> traceSources;
};

// ================================================================================
//  Metrics service
// ================================================================================

struct MetricsSink
{
    enum class Type
    {
        Undefined,
        JsonFile,
        Remote,
    };

    Type type{Type::Undefined};
    std::string name;
};

//! \brief Metrics configuration
struct Metrics
{
    std::vector<MetricsSink> sinks;
    std::optional<bool> collectFromRemote;
    std::chrono::seconds updateInterval{1};
};

// ================================================================================
//  Extensions
// ================================================================================

//! \brief Structure that contains extension settings
struct Extensions
{
    //! \brief Additional search path hints for extensions
    std::vector<std::string> searchPathHints;
};

// ================================================================================
//  VAsio Middleware
// ================================================================================

struct Middleware
{
    std::string registryUri{}; //!< Registry URI to connect to (configuration has priority)
    int connectAttempts{1};    //!<  Number of connection attempts to the registry a participant should perform.
    int tcpReceiveBufferSize{-1};
    int tcpSendBufferSize{-1};
    bool tcpNoDelay{
        true}; //!< Setting this option to true disables Nagle's algorithm on all TCP/IP sockets. Defaults to true for performance reasons.
    bool tcpQuickAck{false};        //!< Setting this Linux specific flag disables delayed TCP/IP acknowledgements.
    bool enableDomainSockets{true}; //!< By default local domain socket is preferred to TCP/IP sockets.
    std::vector<std::string>
        acceptorUris{}; //!< Explicit list of endpoints this participant will accept connections on.
    //! By default, communication with other participants using the registry as a proxy is enabled.
    bool registryAsFallbackProxy{true};
    //! By default, requesting connection of other participants, and honoring these requests by other participants is enabled.
    bool experimentalRemoteParticipantConnection{true};
    //! Timeout for individual connection attempts (TCP, Local-Domain) and handshakes.
    double connectTimeoutSeconds{5.0};
};


// ================================================================================
//  TimeSynchronization
// ================================================================================

//! \brief Structure that contains experimental TimeSynchronization settings
struct TimeSynchronization
{
    double animationFactor{0.0};
    Aggregation enableMessageAggregation{Aggregation::Off};
};

// ================================================================================
//  Experimental
// ================================================================================

//! \brief Structure that contains experimental settings
struct Experimental
{
    TimeSynchronization timeSynchronization;
    Metrics metrics;
};

// ================================================================================
//  Includes
// ================================================================================

//! \brief Structure that contains experimental settings
struct Includes
{
    std::vector<std::string> searchPathHints;
    std::vector<std::string> files;
};

// ================================================================================
//  Root
// ================================================================================

//! \brief Current Schema version for ParticipantConfigurations
constexpr inline auto GetSchemaVersion() -> const char*
{
    return "1";
}

//! \brief ParticipantConfiguration is the main configuration data object for a SIL Kit participant.
struct ParticipantConfiguration : public IParticipantConfiguration
{
    ParticipantConfiguration() = default;

    //virtual auto ToYamlString() -> std::string override;
    //virtual auto ToJsonString() -> std::string override;

    //! \brief Version of the JSON/YAML schema. Currently is at 1
    std::string schemaVersion{""};
    //! \brief An optional user description for documentation purposes. Currently unused.
    std::string description;
    //! \brief An optional file path.
    std::string configurationFilePath;

    //! \brief The participant name (configuration has priority)
    std::string participantName;

    std::vector<CanController> canControllers;
    std::vector<LinController> linControllers;
    std::vector<EthernetController> ethernetControllers;
    std::vector<FlexrayController> flexrayControllers;

    std::vector<DataPublisher> dataPublishers;
    std::vector<DataSubscriber> dataSubscribers;

    std::vector<RpcServer> rpcServers;
    std::vector<RpcClient> rpcClients;

    Logging logging;
    HealthCheck healthCheck;
    Tracing tracing;
    Extensions extensions;
    Includes includes;
    Middleware middleware;
    Experimental experimental;
};

bool operator==(const CanController& lhs, const CanController& rhs);
bool operator==(const LinController& lhs, const LinController& rhs);
bool operator==(const EthernetController& lhs, const EthernetController& rhs);
bool operator==(const FlexrayController& lhs, const FlexrayController& rhs);
bool operator==(const DataPublisher& lhs, const DataPublisher& rhs);
bool operator==(const DataSubscriber& lhs, const DataSubscriber& rhs);
bool operator==(const RpcServer& lhs, const RpcServer& rhs);
bool operator==(const RpcClient& lhs, const RpcClient& rhs);
bool operator==(const HealthCheck& lhs, const HealthCheck& rhs);
bool operator==(const Tracing& lhs, const Tracing& rhs);
bool operator==(const MetricsSink& lhs, const MetricsSink& rhs);
bool operator==(const Metrics& lhs, const Metrics& rhs);
bool operator==(const Extensions& lhs, const Extensions& rhs);
bool operator==(const Middleware& lhs, const Middleware& rhs);
bool operator==(const Includes& lhs, const Includes& rhs);
bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs);
bool operator==(const TimeSynchronization& lhs, const TimeSynchronization& rhs);
bool operator==(const Experimental& lhs, const Experimental& rhs);
bool operator==(const Label& lhs, const Label& rhs);
bool operator==(const SimulatedNetwork& lhs, const SimulatedNetwork& rhs);

auto operator<<(std::ostream& out, const Label::Kind& kind) -> std::ostream&;
auto operator<<(std::ostream& out, const Label& label) -> std::ostream&;

bool operator<(const MetricsSink& lhs, const MetricsSink& rhs);
bool operator>(const MetricsSink& lhs, const MetricsSink& rhs);

} // namespace V1
} // namespace Config
} // namespace SilKit

namespace SilKitRegistry {
namespace Config {
namespace V1 {

constexpr inline auto GetSchemaVersion() -> const char*
{
    return "1";
}

struct Experimental
{
    SilKit::Config::V1::Metrics metrics;
};

struct RegistryConfiguration
{
    std::string description{""};
    std::string schemaVersion{""};
    std::optional<std::string> listenUri;
    std::optional<bool> enableDomainSockets;
    std::optional<std::string> dashboardUri;
    SilKit::Config::Logging logging{};
    Experimental experimental{};
};

bool operator==(const Experimental& lhs, const Experimental& rhs);

} // namespace V1
} // namespace Config
} // namespace SilKitRegistry
