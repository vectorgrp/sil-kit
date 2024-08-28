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

#pragma once

#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/services/flexray/FlexrayDatatypes.hpp"
#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

#include "Configuration.hpp"
#include "Optional.hpp"

namespace SilKit {
namespace Config {
inline namespace v1 {

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
    SilKit::Util::Optional<std::string> network;
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
    SilKit::Util::Optional<std::string> network;

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
    SilKit::Util::Optional<std::string> network;

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
    SilKit::Util::Optional<std::string> network;

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
    SilKit::Util::Optional<std::string> network;

    SilKit::Util::Optional<Services::Flexray::FlexrayClusterParameters> clusterParameters;
    SilKit::Util::Optional<Services::Flexray::FlexrayNodeParameters> nodeParameters;
    std::vector<Services::Flexray::FlexrayTxBufferConfig> txBufferConfigurations;

    std::vector<std::string> useTraceSinks;
    Replay replay;
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
    SilKit::Util::Optional<std::string> topic;

    //! \brief History length of a DataPublisher.
    SilKit::Util::Optional<size_t> history{0};

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
    SilKit::Util::Optional<std::string> topic;

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
    SilKit::Util::Optional<std::string> functionName;

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
    SilKit::Util::Optional<std::string> functionName;

    std::vector<std::string> useTraceSinks;
    Replay replay;
};

// ================================================================================
//  Health checking service
// ================================================================================

//! \brief Health checking service
struct HealthCheck
{
    SilKit::Util::Optional<std::chrono::milliseconds> softResponseTimeout;
    SilKit::Util::Optional<std::chrono::milliseconds> hardResponseTimeout;
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
    bool collectFromRemote{false};
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
    int connectAttempts{1}; //!<  Number of connection attempts to the registry a participant should perform.
    int tcpReceiveBufferSize{-1};
    int tcpSendBufferSize{-1};
    bool tcpNoDelay{false}; //!< Disables Nagle's algorithm.
    bool tcpQuickAck{false}; //!< Setting this Linux specific flag disables delayed TCP/IP acknowledgements.
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
//  Root
// ================================================================================

//! \brief ParticipantConfiguration is the main configuration data object for a SILKIT participant.
struct ParticipantConfiguration : public IParticipantConfiguration
{
    ParticipantConfiguration() = default;

    //virtual auto ToYamlString() -> std::string override;
    //virtual auto ToJsonString() -> std::string override;

    //! \brief Version of the JSON/YAML schema.
    std::string schemaVersion{"1"};
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
bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs);
bool operator==(const TimeSynchronization& lhs, const TimeSynchronization& rhs);
bool operator==(const Experimental& lhs, const Experimental& rhs);

bool operator<(const MetricsSink& lhs, const MetricsSink& rhs);
bool operator>(const MetricsSink& lhs, const MetricsSink& rhs);

} // namespace v1
} // namespace Config
} // namespace SilKit
